// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/moduleparam.h>
#include <linux/kprobes.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include "xhci.h"
#include "xhci-trace.h"
#ifdef CONFIG_USB_XHCI_HCD
static int usb_debug;
module_param(usb_debug, int, 0644);

static int (*aml_xhci_ring_expansion)(struct xhci_hcd *xhci, struct xhci_ring *ring,
						unsigned int num_trbs, gfp_t flags);

static void (*aml_queue_trb) (struct xhci_hcd *xhci, struct xhci_ring *ring,
		bool more_trbs_coming,
		u32 field1, u32 field2, u32 field3, u32 field4);

static void (* aml_xhci_ring_ep_doorbell) (struct xhci_hcd *xhci,
                unsigned int slot_id,
                unsigned int ep_index,
                unsigned int stream_id);

static struct xhci_ring * (*aml_xhci_triad_to_transfer_ring) (struct xhci_hcd *xhci,
                unsigned int slot_id, unsigned int ep_index,
                unsigned int stream_id);

static int prepare_transfer(struct xhci_hcd *xhci,
		struct xhci_virt_device *xdev,
		unsigned int ep_index,
		unsigned int stream_id,
		unsigned int num_trbs,
		struct urb *urb,
		unsigned int td_index,
		gfp_t mem_flags);

static unsigned int (*aml_xhci_get_endpoint_index) (struct usb_endpoint_descriptor *desc);

static bool aml_xhci_urb_suitable_for_idt(struct urb *urb)
{
	if (!usb_endpoint_xfer_isoc(&urb->ep->desc) && usb_urb_dir_out(urb) &&
	    usb_endpoint_maxp(&urb->ep->desc) >= TRB_IDT_MAX_SIZE &&
	    urb->transfer_buffer_length <= TRB_IDT_MAX_SIZE &&
	    !(urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP) &&
	    !urb->num_sgs) {
		if (urb->transfer_buffer_length == 0)
			return false;
		return true;
	}
	return false;
}

static inline __nocfi struct xhci_ring *aml_xhci_urb_to_transfer_ring(struct xhci_hcd *xhci,
                                                                struct urb *urb)
{
        return aml_xhci_triad_to_transfer_ring(xhci, urb->dev->slot_id,
                                        aml_xhci_get_endpoint_index(&urb->ep->desc),
                                        urb->stream_id);
}

static __nocfi unsigned int aml_count_trbs(u64 addr, u64 len)
{
	unsigned int num_trbs;

	num_trbs = DIV_ROUND_UP(len + (addr & (TRB_MAX_BUFF_SIZE - 1)),
			TRB_MAX_BUFF_SIZE);
	if (num_trbs == 0)
		num_trbs++;

	return num_trbs;
}

static __nocfi bool trb_is_link(union xhci_trb *trb)
{
	return TRB_TYPE_LINK_LE32(trb->link.control);
}

static inline unsigned int count_trbs_needed(struct urb *urb)
{
	return aml_count_trbs(urb->transfer_dma, urb->transfer_buffer_length);
}

static __nocfi unsigned int count_sg_trbs_needed(struct urb *urb)
{
	struct scatterlist *sg;
	unsigned int i, len, full_len, num_trbs = 0;

	full_len = urb->transfer_buffer_length;

	for_each_sg(urb->sg, sg, urb->num_mapped_sgs, i) {
		len = sg_dma_len(sg);
		num_trbs += aml_count_trbs(sg_dma_address(sg), len);
		len = min_t(unsigned int, len, full_len);
		full_len -= len;
		if (full_len == 0)
			break;
	}

	return num_trbs;
}

static __nocfi int xhci_align_td(struct xhci_hcd *xhci, struct urb *urb, u32 enqd_len,
			 u32 *trb_buff_len, struct xhci_segment *seg)
{
	struct device *dev = xhci_to_hcd(xhci)->self.controller;
	unsigned int unalign;
	unsigned int max_pkt;
	u32 new_buff_len;
	size_t len;

	max_pkt = usb_endpoint_maxp(&urb->ep->desc);
	unalign = (enqd_len + *trb_buff_len) % max_pkt;

	/* we got lucky, last normal TRB data on segment is packet aligned */
	if (unalign == 0)
		return 0;

	xhci_dbg(xhci, "Unaligned %d bytes, buff len %d\n",
		 unalign, *trb_buff_len);

	/* is the last nornal TRB alignable by splitting it */
	if (*trb_buff_len > unalign) {
		*trb_buff_len -= unalign;
		xhci_dbg(xhci, "split align, new buff len %d\n", *trb_buff_len);
		return 0;
	}

	/*
	 * We want enqd_len + trb_buff_len to sum up to a number aligned to
	 * number which is divisible by the endpoint's wMaxPacketSize. IOW:
	 * (size of currently enqueued TRBs + remainder) % wMaxPacketSize == 0.
	 */
	new_buff_len = max_pkt - (enqd_len % max_pkt);

	if (new_buff_len > (urb->transfer_buffer_length - enqd_len))
		new_buff_len = (urb->transfer_buffer_length - enqd_len);

	/* create a max max_pkt sized bounce buffer pointed to by last trb */
	if (usb_urb_dir_out(urb)) {
		if (urb->num_sgs) {
			len = sg_pcopy_to_buffer(urb->sg, urb->num_sgs,
						 seg->bounce_buf, new_buff_len, enqd_len);
			if (len != new_buff_len)
				xhci_warn(xhci, "WARN Wrong bounce buffer write length: %zu != %d\n",
					  len, new_buff_len);
		} else {
			memcpy(seg->bounce_buf, urb->transfer_buffer + enqd_len, new_buff_len);
		}

		seg->bounce_dma = dma_map_single(dev, seg->bounce_buf,
						 max_pkt, DMA_TO_DEVICE);
	} else {
		seg->bounce_dma = dma_map_single(dev, seg->bounce_buf,
						 max_pkt, DMA_FROM_DEVICE);
	}

	if (dma_mapping_error(dev, seg->bounce_dma)) {
		/* try without aligning. Some host controllers survive */
		xhci_warn(xhci, "Failed mapping bounce buffer, not aligning\n");
		return 0;
	}
	*trb_buff_len = new_buff_len;
	seg->bounce_len = new_buff_len;
	seg->bounce_offs = enqd_len;

	xhci_dbg(xhci, "Bounce align, new buff len %d\n", *trb_buff_len);

	return 1;
}

static __nocfi void giveback_first_trb(struct xhci_hcd *xhci, int slot_id,
		unsigned int ep_index, unsigned int stream_id, int start_cycle,
		struct xhci_generic_trb *start_trb)
{
	/*
	 * Pass all the TRBs to the hardware at once and make sure this write
	 * isn't reordered.
	 */
	wmb();
	if (start_cycle)
		start_trb->field[3] |= cpu_to_le32(start_cycle);
	else
		start_trb->field[3] &= cpu_to_le32(~TRB_CYCLE);
	aml_xhci_ring_ep_doorbell(xhci, slot_id, ep_index, stream_id);
}
static __nocfi void check_trb_math(struct urb *urb, int running_total)
{
	if (unlikely(running_total != urb->transfer_buffer_length))
		dev_err(&urb->dev->dev, "%s - ep %#x - Miscalculated tx length, "
				"queued %#x (%d), asked for %#x (%d)\n",
				__func__,
				urb->ep->desc.bEndpointAddress,
				running_total, running_total,
				urb->transfer_buffer_length,
				urb->transfer_buffer_length);
}

static __nocfi u32 xhci_td_remainder(struct xhci_hcd *xhci, int transferred,
			      int trb_buff_len, unsigned int td_total_len,
			      struct urb *urb, bool more_trbs_coming)
{
	u32 maxp, total_packet_count;

	/* MTK xHCI 0.96 contains some features from 1.0 */
	if (xhci->hci_version < 0x100 && !(xhci->quirks & XHCI_MTK_HOST))
		return ((td_total_len - transferred) >> 10);

	/* One TRB with a zero-length data packet. */
	if (!more_trbs_coming || (transferred == 0 && trb_buff_len == 0) ||
	    trb_buff_len == td_total_len)
		return 0;

	/* for MTK xHCI 0.96, TD size include this TRB, but not in 1.x */
	if ((xhci->quirks & XHCI_MTK_HOST) && (xhci->hci_version < 0x100))
		trb_buff_len = 0;

	maxp = usb_endpoint_maxp(&urb->ep->desc);
	total_packet_count = DIV_ROUND_UP(td_total_len, maxp);

	/* Queueing functions don't count the current TRB into transferred */
	return (total_packet_count - ((transferred + trb_buff_len) / maxp));
}

static bool xhci_urb_temp_buffer_required(struct usb_hcd *hcd,
					  struct urb *urb)
{
	bool ret = false;
	unsigned int i;
	unsigned int len = 0;
	unsigned int trb_size;
	unsigned int max_pkt;
	struct scatterlist *sg;
	struct scatterlist *tail_sg;

	tail_sg = urb->sg;
	max_pkt = usb_endpoint_maxp(&urb->ep->desc);

	if (!urb->num_sgs)
		return ret;

	if (urb->dev->speed >= USB_SPEED_SUPER)
		trb_size = TRB_CACHE_SIZE_SS;
	else
		trb_size = TRB_CACHE_SIZE_HS;

	if (urb->transfer_buffer_length != 0 &&
	    !(urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP)) {
		for_each_sg(urb->sg, sg, urb->num_sgs, i) {
			len = len + sg->length;
			if (i > trb_size - 2) {
				len = len - tail_sg->length;
				if (len < max_pkt) {
					ret = true;
					break;
				}

				tail_sg = sg_next(tail_sg);
			}
		}
	}
	return ret;
}

static int xhci_map_temp_buffer(struct usb_hcd *hcd, struct urb *urb)
{
	void *temp;
	int ret = 0;
	unsigned int buf_len;
	enum dma_data_direction dir;

	dir = usb_urb_dir_in(urb) ? DMA_FROM_DEVICE : DMA_TO_DEVICE;
	buf_len = urb->transfer_buffer_length;

	temp = kzalloc_node(buf_len, GFP_ATOMIC,
			    dev_to_node(hcd->self.sysdev));

	if (usb_urb_dir_out(urb))
		sg_pcopy_to_buffer(urb->sg, urb->num_sgs,
				   temp, buf_len, 0);

	urb->transfer_buffer = temp;
	urb->transfer_dma = dma_map_single(hcd->self.sysdev,
					   urb->transfer_buffer,
					   urb->transfer_buffer_length,
					   dir);

	if (dma_mapping_error(hcd->self.sysdev,
			      urb->transfer_dma)) {
		ret = -EAGAIN;
		kfree(temp);
	} else {
		urb->transfer_flags |= URB_DMA_MAP_SINGLE;
	}

	return ret;
}

static bool link_trb_toggles_cycle(union xhci_trb *trb)
{
	return le32_to_cpu(trb->link.control) & LINK_TOGGLE;
}

static bool last_trb_on_seg(struct xhci_segment *seg, union xhci_trb *trb)
{
	return trb == &seg->trbs[TRBS_PER_SEGMENT - 1];
}

static inline int room_on_ring(struct xhci_hcd *xhci, struct xhci_ring *ring,
		unsigned int num_trbs)
{
	int num_trbs_in_deq_seg;

	if (ring->num_trbs_free < num_trbs)
		return 0;

	if (ring->type != TYPE_COMMAND && ring->type != TYPE_EVENT) {
		num_trbs_in_deq_seg = ring->dequeue - ring->deq_seg->trbs;
		if (ring->num_trbs_free < num_trbs + num_trbs_in_deq_seg)
			return 0;
	}

	return 1;
}

static int __nocfi prepare_ring(struct xhci_hcd *xhci, struct xhci_ring *ep_ring,
		u32 ep_state, unsigned int num_trbs, gfp_t mem_flags)
{
	unsigned int num_trbs_needed;
	unsigned int link_trb_count = 0;

	/* Make sure the endpoint has been added to xHC schedule */
	switch (ep_state) {
	case EP_STATE_DISABLED:
		/*
		 * USB core changed config/interfaces without notifying us,
		 * or hardware is reporting the wrong state.
		 */
		xhci_warn(xhci, "WARN urb submitted to disabled ep\n");
		return -ENOENT;
	case EP_STATE_ERROR:
		xhci_warn(xhci, "WARN waiting for error on ep to be cleared\n");
		/* FIXME event handling code for error needs to clear it */
		/* XXX not sure if this should be -ENOENT or not */
		return -EINVAL;
	case EP_STATE_HALTED:
		xhci_dbg(xhci, "WARN halted endpoint, queueing URB anyway.\n");
		break;
	case EP_STATE_STOPPED:
	case EP_STATE_RUNNING:
		break;
	default:
		xhci_err(xhci, "ERROR unknown endpoint state for ep\n");
		/*
		 * FIXME issue Configure Endpoint command to try to get the HC
		 * back into a known state.
		 */
		return -EINVAL;
	}

	while (1) {
		if (room_on_ring(xhci, ep_ring, num_trbs))
			break;

		if (ep_ring == xhci->cmd_ring) {
			xhci_err(xhci, "Do not support expand command ring\n");
			return -ENOMEM;
		}

//		xhci_dbg_trace(xhci, trace_xhci_dbg_ring_expansion,
//				"ERROR no room on ep ring, try ring expansion");
		num_trbs_needed = num_trbs - ep_ring->num_trbs_free;
		if (aml_xhci_ring_expansion(xhci, ep_ring, num_trbs_needed,
					mem_flags)) {
			xhci_err(xhci, "Ring expansion failed\n");
			return -ENOMEM;
		}
	}

	while (trb_is_link(ep_ring->enqueue)) {
		/* If we're not dealing with 0.95 hardware or isoc rings
		 * on AMD 0.96 host, clear the chain bit.
		 */
		if (!xhci_link_trb_quirk(xhci) &&
		    !(ep_ring->type == TYPE_ISOC &&
		      (xhci->quirks & XHCI_AMD_0x96_HOST)))
			ep_ring->enqueue->link.control &=
				cpu_to_le32(~TRB_CHAIN);
		else
			ep_ring->enqueue->link.control |=
				cpu_to_le32(TRB_CHAIN);

		wmb();
		ep_ring->enqueue->link.control ^= cpu_to_le32(TRB_CYCLE);

		/* Toggle the cycle bit after the last ring segment. */
		if (link_trb_toggles_cycle(ep_ring->enqueue))
			ep_ring->cycle_state ^= 1;

		ep_ring->enq_seg = ep_ring->enq_seg->next;
		ep_ring->enqueue = ep_ring->enq_seg->trbs;

		/* prevent infinite loop if all first trbs are link trbs */
		if (link_trb_count++ > ep_ring->num_segs) {
			xhci_warn(xhci, "Ring is an endless link TRB loop\n");
			return -EINVAL;
		}
	}

	if (last_trb_on_seg(ep_ring->enq_seg, ep_ring->enqueue)) {
		xhci_warn(xhci, "Missing link TRB at end of ring segment\n");
		return -EINVAL;
	}

	return 0;
}

static int __nocfi prepare_transfer(struct xhci_hcd *xhci,
		struct xhci_virt_device *xdev,
		unsigned int ep_index,
		unsigned int stream_id,
		unsigned int num_trbs,
		struct urb *urb,
		unsigned int td_index,
		gfp_t mem_flags)
{
	int ret;
	struct urb_priv *urb_priv;
	struct xhci_td	*td;
	struct xhci_ring *ep_ring;
	struct xhci_ep_ctx *ep_ctx = xhci_get_ep_ctx(xhci, xdev->out_ctx, ep_index);

	ep_ring = aml_xhci_triad_to_transfer_ring(xhci, xdev->slot_id, ep_index,
					      stream_id);
	if (!ep_ring) {
		xhci_dbg(xhci, "Can't prepare ring for bad stream ID %u\n",
				stream_id);
		return -EINVAL;
	}

	ret = prepare_ring(xhci, ep_ring, GET_EP_CTX_STATE(ep_ctx),
			   num_trbs, mem_flags);
	if (ret)
		return ret;

	urb_priv = urb->hcpriv;
	td = &urb_priv->td[td_index];

	INIT_LIST_HEAD(&td->td_list);
	INIT_LIST_HEAD(&td->cancelled_td_list);

	if (td_index == 0) {
		ret = usb_hcd_link_urb_to_ep(bus_to_hcd(urb->dev->bus), urb);
		if (unlikely(ret))
			return ret;
	}

	td->urb = urb;
	/* Add this TD to the tail of the endpoint ring's TD list */
	list_add_tail(&td->td_list, &ep_ring->td_list);
	td->start_seg = ep_ring->enq_seg;
	td->first_trb = ep_ring->enqueue;

	return 0;
}


/* This is very similar to what ehci-q.c qtd_fill() does */
int __nocfi aml_xhci_queue_bulk_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ring *ring;
	struct urb_priv *urb_priv;
	struct xhci_td *td;
	struct xhci_generic_trb *start_trb;
	struct scatterlist *sg = NULL;
	bool more_trbs_coming = true;
	bool need_zero_pkt = false;
	bool first_trb = true;
	unsigned int num_trbs;
	unsigned int start_cycle, num_sgs = 0;
	unsigned int enqd_len, block_len, trb_buff_len, full_len;
	int sent_len, ret;
	u32 field, length_field, remainder;
	u64 addr, send_addr;

	if (usb_debug)
		pr_info("%s():%d\n", __func__, __LINE__);

	ring = aml_xhci_urb_to_transfer_ring(xhci, urb);
	if (!ring)
		return -EINVAL;

	full_len = urb->transfer_buffer_length;
	/* If we have scatter/gather list, we use it. */
	if (urb->num_sgs && !(urb->transfer_flags & URB_DMA_MAP_SINGLE)) {
		num_sgs = urb->num_mapped_sgs;
		sg = urb->sg;
		addr = (u64) sg_dma_address(sg);
		block_len = sg_dma_len(sg);
		num_trbs = count_sg_trbs_needed(urb);
	} else {
		num_trbs = count_trbs_needed(urb);
		addr = (u64) urb->transfer_dma;
		block_len = full_len;
	}
	ret = prepare_transfer(xhci, xhci->devs[slot_id],
			ep_index, urb->stream_id,
			num_trbs, urb, 0, mem_flags);
	if (unlikely(ret < 0))
		return ret;

	urb_priv = urb->hcpriv;

	/* Deal with URB_ZERO_PACKET - need one more td/trb */
	if (urb->transfer_flags & URB_ZERO_PACKET && urb_priv->num_tds > 1)
		need_zero_pkt = true;

	td = &urb_priv->td[0];

	/*
	 * Don't give the first TRB to the hardware (by toggling the cycle bit)
	 * until we've finished creating all the other TRBs.  The ring's cycle
	 * state may change as we enqueue the other TRBs, so save it too.
	 */
	start_trb = &ring->enqueue->generic;
	start_cycle = ring->cycle_state;
	send_addr = addr;

	/* Queue the TRBs, even if they are zero-length */
	for (enqd_len = 0; first_trb || enqd_len < full_len;
			enqd_len += trb_buff_len) {
		field = TRB_TYPE(TRB_NORMAL);

		/* TRB buffer should not cross 64KB boundaries */
		trb_buff_len = TRB_BUFF_LEN_UP_TO_BOUNDARY(addr);
		trb_buff_len = min_t(unsigned int, trb_buff_len, block_len);

		if (enqd_len + trb_buff_len > full_len)
			trb_buff_len = full_len - enqd_len;

		/* Don't change the cycle bit of the first TRB until later */
		if (first_trb) {
			first_trb = false;
			if (start_cycle == 0)
				field |= TRB_CYCLE;
		} else
			field |= ring->cycle_state;

		/* Chain all the TRBs together; clear the chain bit in the last
		 * TRB to indicate it's the last TRB in the chain.
		 */
		if (enqd_len + trb_buff_len < full_len) {
			field |= TRB_CHAIN;
			if (trb_is_link(ring->enqueue + 1)) {
				if (xhci_align_td(xhci, urb, enqd_len,
						  &trb_buff_len,
						  ring->enq_seg)) {
					send_addr = ring->enq_seg->bounce_dma;
					/* assuming TD won't span 2 segs */
					td->bounce_seg = ring->enq_seg;
				}
			}
		}
		if (enqd_len + trb_buff_len >= full_len) {
			field &= ~TRB_CHAIN;
			field |= TRB_IOC;
			more_trbs_coming = false;
			td->last_trb = ring->enqueue;
			td->last_trb_seg = ring->enq_seg;
			if (aml_xhci_urb_suitable_for_idt(urb)) {
				memcpy(&send_addr, urb->transfer_buffer,
				       trb_buff_len);
				le64_to_cpus(&send_addr);
				field |= TRB_IDT;
			}
		}

		/* Only set interrupt on short packet for IN endpoints */
		if (usb_urb_dir_in(urb))
			field |= TRB_ISP;

		/* Set the TRB length, TD size, and interrupter fields. */
		remainder = xhci_td_remainder(xhci, enqd_len, trb_buff_len,
					      full_len, urb, more_trbs_coming);

		length_field = TRB_LEN(trb_buff_len) |
			TRB_TD_SIZE(remainder) |
			TRB_INTR_TARGET(0);

		aml_queue_trb(xhci, ring, more_trbs_coming | need_zero_pkt,
				lower_32_bits(send_addr),
				upper_32_bits(send_addr),
				length_field,
				field);
		td->num_trbs++;
		addr += trb_buff_len;
		sent_len = trb_buff_len;

		while (sg && sent_len >= block_len) {
			/* New sg entry */
			--num_sgs;
			sent_len -= block_len;
			sg = sg_next(sg);
			if (num_sgs != 0 && sg) {
				block_len = sg_dma_len(sg);
				addr = (u64) sg_dma_address(sg);
				addr += sent_len;
			}
		}
		block_len -= sent_len;
		send_addr = addr;
	}

	if (need_zero_pkt) {
		ret = prepare_transfer(xhci, xhci->devs[slot_id],
				       ep_index, urb->stream_id,
				       1, urb, 1, mem_flags);
		urb_priv->td[1].last_trb = ring->enqueue;
		urb_priv->td[1].last_trb_seg = ring->enq_seg;
		field = TRB_TYPE(TRB_NORMAL) | ring->cycle_state | TRB_IOC;
		aml_queue_trb(xhci, ring, 0, 0, 0, TRB_INTR_TARGET(0), field);
		urb_priv->td[1].num_trbs++;
	}

	check_trb_math(urb, enqd_len);
	giveback_first_trb(xhci, slot_id, ep_index, urb->stream_id,
			start_cycle, start_trb);
	return 0;
}


/* Caller must have locked xhci->lock */
int __nocfi aml_xhci_queue_ctrl_tx(struct xhci_hcd *xhci, gfp_t mem_flags,
		struct urb *urb, int slot_id, unsigned int ep_index)
{
	struct xhci_ring *ep_ring;
	int num_trbs;
	int ret;
	struct usb_ctrlrequest *setup;
	struct xhci_generic_trb *start_trb;
	int start_cycle;
	u32 field;
	struct urb_priv *urb_priv;
	struct xhci_td *td;

	if (usb_debug)
		pr_info("%s():%d\n", __func__, __LINE__);

	ep_ring = aml_xhci_urb_to_transfer_ring(xhci, urb);
	if (!ep_ring)
		return -EINVAL;

	/*
	 * Need to copy setup packet into setup TRB, so we can't use the setup
	 * DMA address.
	 */
	if (!urb->setup_packet)
		return -EINVAL;

	/* 1 TRB for setup, 1 for status */
	num_trbs = 2;
	/*
	 * Don't need to check if we need additional event data and normal TRBs,
	 * since data in control transfers will never get bigger than 16MB
	 * XXX: can we get a buffer that crosses 64KB boundaries?
	 */
	if (urb->transfer_buffer_length > 0)
		num_trbs++;
	ret = prepare_transfer(xhci, xhci->devs[slot_id],
			ep_index, urb->stream_id,
			num_trbs, urb, 0, mem_flags);
	if (ret < 0)
		return ret;

	urb_priv = urb->hcpriv;
	td = &urb_priv->td[0];
	td->num_trbs = num_trbs;

	/*
	 * Don't give the first TRB to the hardware (by toggling the cycle bit)
	 * until we've finished creating all the other TRBs.  The ring's cycle
	 * state may change as we enqueue the other TRBs, so save it too.
	 */
	start_trb = &ep_ring->enqueue->generic;
	start_cycle = ep_ring->cycle_state;

	/* Queue setup TRB - see section 6.4.1.2.1 */
	/* FIXME better way to translate setup_packet into two u32 fields? */
	setup = (struct usb_ctrlrequest *) urb->setup_packet;
	field = 0;
	field |= TRB_IDT | TRB_TYPE(TRB_SETUP);
	if (start_cycle == 0)
		field |= 0x1;

	/* xHCI 1.0/1.1 6.4.1.2.1: Transfer Type field */
	if ((xhci->hci_version >= 0x100) || (xhci->quirks & XHCI_MTK_HOST)) {
		if (urb->transfer_buffer_length > 0) {
			if (setup->bRequestType & USB_DIR_IN)
				field |= TRB_TX_TYPE(TRB_DATA_IN);
			else
				field |= TRB_TX_TYPE(TRB_DATA_OUT);
		}
	}

	aml_queue_trb(xhci, ep_ring, true,
		  setup->bRequestType | setup->bRequest << 8 | le16_to_cpu(setup->wValue) << 16,
		  le16_to_cpu(setup->wIndex) | le16_to_cpu(setup->wLength) << 16,
		  TRB_LEN(8) | TRB_INTR_TARGET(0),
		  /* Immediate data in pointer */
		  field);

	/* If there's data, queue data TRBs */
	/* Only set interrupt on short packet for IN endpoints */
	if (usb_urb_dir_in(urb))
		field = TRB_ISP | TRB_TYPE(TRB_DATA);
	else
		field = TRB_TYPE(TRB_DATA);

	if (urb->transfer_buffer_length > 0) {
		u32 length_field, remainder;
		u64 addr;

		if (aml_xhci_urb_suitable_for_idt(urb)) {
			memcpy(&addr, urb->transfer_buffer,
			       urb->transfer_buffer_length);
			le64_to_cpus(&addr);
			field |= TRB_IDT;
		} else {
			addr = (u64) urb->transfer_dma;
		}

		remainder = xhci_td_remainder(xhci, 0,
				urb->transfer_buffer_length,
				urb->transfer_buffer_length,
				urb, 1);
		length_field = TRB_LEN(urb->transfer_buffer_length) |
				TRB_TD_SIZE(remainder) |
				TRB_INTR_TARGET(0);
		if (setup->bRequestType & USB_DIR_IN)
			field |= TRB_DIR_IN;
		aml_queue_trb(xhci, ep_ring, true,
				lower_32_bits(addr),
				upper_32_bits(addr),
				length_field,
				field | ep_ring->cycle_state);
	}

	/* Save the DMA address of the last TRB in the TD */
	td->last_trb = ep_ring->enqueue;
	td->last_trb_seg = ep_ring->enq_seg;

	/* Queue status TRB - see Table 7 and sections 4.11.2.2 and 6.4.1.2.3 */
	/* If the device sent data, the status stage is an OUT transfer */
	if (urb->transfer_buffer_length > 0 && setup->bRequestType & USB_DIR_IN)
		field = 0;
	else
		field = TRB_DIR_IN;
	aml_queue_trb(xhci, ep_ring, false,
			0,
			0,
			TRB_INTR_TARGET(0),
			/* Event on completion */
			field | TRB_IOC | TRB_TYPE(TRB_STATUS) | ep_ring->cycle_state);

	giveback_first_trb(xhci, slot_id, ep_index, 0,
			start_cycle, start_trb);
	return 0;
}

int __nocfi aml_xhci_map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb,
				gfp_t mem_flags)
{
	struct xhci_hcd *xhci;

	if (usb_debug)
		pr_info("%s():%d\n", __func__, __LINE__);

	xhci = hcd_to_xhci(hcd);

	if (aml_xhci_urb_suitable_for_idt(urb))
		return 0;

	if (xhci->quirks & XHCI_SG_TRB_CACHE_SIZE_QUIRK) {
		if (xhci_urb_temp_buffer_required(hcd, urb))
			return xhci_map_temp_buffer(hcd, urb);
	}
	return usb_hcd_map_urb_for_dma(hcd, urb, mem_flags);
}

static void *get_symbol_addr(const char *symbol_name)
{
	struct kprobe kp = {
		.symbol_name = symbol_name,
	};
	int ret;

	ret = register_kprobe(&kp);
	if (ret < 0) {
		pr_err("register_kprobe:%s failed, returned %d\n", symbol_name, ret);
		return NULL;
	}
	pr_debug("symbol_name:%s addr=%px\n", symbol_name, kp.addr);
	unregister_kprobe(&kp);

	return kp.addr;
}

static int __nocfi __kprobes xhci_queue_bulk_tx_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	/*
	int ret;

	ret = aml_xhci_queue_bulk_tx((struct xhci_hcd *)regs->regs[0],
							(gfp_t)regs->regs[1],
							(struct urb *)regs->regs[2],
							(int)regs->regs[3],
							(unsigned int)regs->regs[4]);

	regs->regs[0] = ret;
	regs->pc = regs->regs[30];
	*/

	//restore to origin context
	instruction_pointer_set(regs, (unsigned long)aml_xhci_queue_bulk_tx);

	//no need continue do single-step
	return 1;
}

struct kprobe kp_xhci_queue_bulk_tx = {
	.symbol_name  = "xhci_queue_bulk_tx",
	.pre_handler = xhci_queue_bulk_tx_pre_handler,
};

static int __nocfi __kprobes xhci_map_urb_for_dma_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	/*
	int ret;

	ret = aml_xhci_map_urb_for_dma((struct usb_hcd *)regs->regs[0],
							(struct urb *)regs->regs[1],
							(gfp_t)regs->regs[2]);

	regs->regs[0] = ret;
	regs->pc = regs->regs[30];
	*/

	//restore to origin context
	instruction_pointer_set(regs, (unsigned long)aml_xhci_map_urb_for_dma);

	//no need continue do single-step
	return 1;
}

struct kprobe kp_xhci_map_urb_for_dma = {
	.symbol_name  = "xhci_map_urb_for_dma",
	.pre_handler = xhci_map_urb_for_dma_pre_handler,
};

static int __nocfi __kprobes xhci_queue_ctrl_tx_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	/*
	int ret;

	ret = aml_xhci_queue_ctrl_tx((struct xhci_hcd *)regs->regs[0],
							(gfp_t)regs->regs[1],
							(struct urb *)regs->regs[2],
							(int)regs->regs[3],
							(unsigned int)regs->regs[4]);

	regs->regs[0] = ret;
	regs->pc = regs->regs[30];
	*/

	//restore to origin context
	instruction_pointer_set(regs, (unsigned long)aml_xhci_queue_ctrl_tx);

	//no need continue do single-step
	return 1;
}

struct kprobe kp_xhci_queue_ctrl_tx = {
	.symbol_name  = "xhci_queue_ctrl_tx",
	.pre_handler = xhci_queue_ctrl_tx_pre_handler,
};

static int first_init = 1;
#endif
int crg_xhci_init(void)
{
#ifdef CONFIG_USB_XHCI_HCD
	int ret;

	if (first_init == 0)
		return 0;

	aml_xhci_ring_expansion = get_symbol_addr("xhci_ring_expansion");
	aml_queue_trb = get_symbol_addr("queue_trb");
	aml_xhci_ring_ep_doorbell = get_symbol_addr("xhci_ring_ep_doorbell");
	aml_xhci_triad_to_transfer_ring = get_symbol_addr("xhci_triad_to_transfer_ring");
	aml_xhci_get_endpoint_index = get_symbol_addr("xhci_get_endpoint_index");

	if (!aml_xhci_ring_expansion || !aml_queue_trb || !aml_xhci_ring_ep_doorbell ||
	    !aml_xhci_triad_to_transfer_ring || !aml_xhci_get_endpoint_index) {
		pr_err("crg_xhci: get symbol failed\n");
		return 1;
	}

	ret = register_kprobe(&kp_xhci_queue_bulk_tx);
	if (ret < 0) {
		pr_err("register_kprobe:%s failed, returned %d\n",
		       kp_xhci_queue_bulk_tx.symbol_name, ret);
		return 1;
	}

	ret = register_kprobe(&kp_xhci_queue_ctrl_tx);
	if (ret < 0) {
		pr_err("register_kprobe:%s failed, returned %d\n",
		       kp_xhci_queue_ctrl_tx.symbol_name, ret);
		unregister_kprobe(&kp_xhci_queue_bulk_tx);
		return 1;
	}

	ret = register_kprobe(&kp_xhci_map_urb_for_dma);
	if (ret < 0) {
		pr_err("register_kprobe:%s failed, returned %d\n",
		       kp_xhci_map_urb_for_dma.symbol_name, ret);
		unregister_kprobe(&kp_xhci_queue_bulk_tx);
		unregister_kprobe(&kp_xhci_queue_ctrl_tx);
		return 1;
	}

	first_init = 0;
#endif
	return 0;
}

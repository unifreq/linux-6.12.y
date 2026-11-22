/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef __USB_V2_C2_HEADER_
#define __USB_V2_C2_HEADER_

#define USB_PHY2_ENABLE			(0x1 << 1)
#define USB_PHY2_RESET			(0x1 << 0)
#define USBPLL_LK_OD_EN			(0x1 << 16)
#define USBPLL_LOCKFLAG_BIT      (31)

#define HOST_MODE	0
#define DEVICE_MODE	1
#define USB_PHY_VERSION_C2		1

union usb_top_version_reg {
	u32 d32;
	struct {
		unsigned phy_version:4;
		unsigned USB20_port_num:4;
		unsigned USB30_port_num:4;
		unsigned BC_version:4;
		unsigned CC_version:4;
		unsigned reserved:12;
	} b;
};

union usb_r6_v2 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned vbusdig_sync:1;
		unsigned vbusdig_reg:1;
		unsigned vbusdig_cfg:2;
		unsigned vbusdig_en0:1;
		unsigned vbusdig_en1:1;
		unsigned vbusdig_curr:1;
		unsigned usb_vbusdig_irq:1;
		unsigned vbusdig_th:8;
		unsigned vbusdig_cnt:8;
		unsigned reserved:8;
	} b;
};

#ifdef CONFIG_AMLOGIC_USBPHYC2
void set_usb_phy_reg10(int mode);
#endif
#endif

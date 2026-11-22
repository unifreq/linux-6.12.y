/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#include <linux/workqueue.h>

struct bc_regs {
	void __iomem	 *bc_r[2];
};

union bc_r0 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned bc_det_sw:1;
		unsigned bc_int_clean:1;
		unsigned bc_en:1;
		unsigned reserved:29;
	} b;
};

union bc_r1 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned bc_det_end:1;
		unsigned port_status:4;
		unsigned bc_state:3;
		unsigned analog_output_signal:8;
		unsigned reserved:16;
	} b;
};

union bc_r2 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned usb2_bc_bypass:1;
		unsigned usb2_bc_dcd_trim:2;
		unsigned bc_otp_func_en:1;
		unsigned reserved:28;
	} b;
};

union bc_r3 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned usb2_otg_vbusdet_en0:1;
		unsigned usb2_bc_dcd_en0:1;
		unsigned usb2_bc_rpd_en:1;
		unsigned usb2_bc_vdp_en0:1;
		unsigned usb2_bc_chg_det_en0:1;
		unsigned usb2_bc_vdm_en0:1;
		unsigned usb2_bc_dcp_det_en0:1;
		unsigned usb2_otg_aca_en0:1;
		unsigned usb2_bgr_en0:1;
		unsigned usb2_otg_sess_vld0:1;
		unsigned usb2_bc_dcd_det0:1;
		unsigned usb2_bc_chg_det0:1;
		unsigned usb2_bc_dcp_det0:1;
		unsigned usb2_otg_aca_iddig0:1;
		unsigned usb2_otg_vbusdet_en1:1;
		unsigned usb2_bc_dcd_en1:1;
		unsigned usb2_bc_rpd_en1:1;
		unsigned usb2_bc_vdp_en1:1;
		unsigned usb2_bc_chg_det_en1:1;
		unsigned usb2_bc_vdm_en1:1;
		unsigned usb2_bc_dcp_det_en1:1;
		unsigned usb2_otg_aca_en1:1;
		unsigned usb2_bgr_en1:1;
		unsigned usb2_otg_sess_vld1:1;
		unsigned usb2_bc_dcd_det1:1;
		unsigned usb2_bc_chg_det1:2;
		unsigned usb2_bc_dcp_det1:1;
		unsigned usb2_otg_aca_iddig1:3;
		unsigned usb2_bc_rpd_en0:1;
	} b;
};

union bc_r4 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned enable_usb2_otg_sess_vld:1;
		unsigned enable_usb2_bc_dcd_det:1;
		unsigned enable_usb2_bc_chg_det:1;
		unsigned enable_usb2_bc_dcp_det:1;
		unsigned enable_usb2_otg_aca_iddig:1;
		unsigned reserved:27;
	} b;
};

union bc_r5 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned second_cnt1_parameter:6;
		unsigned second_cnt2_parameter:7;
		unsigned first_cnt_parameter:7;
		unsigned aca_cnt_parameter:6;
		unsigned vbus_cnt_parameter:6;
	} b;
};

union bc_r6 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned filter_time_parameter:3;
		unsigned dcd_timer_cnt_parameter:10;
		unsigned reserved:19;
	} b;
};

struct bc_dev {
	struct device	*dev;
	void __iomem	*regs;
	void __iomem	*phy_base;
	struct delayed_work	work;
	struct delayed_work	vbus_work;
	u32 bc_status;
};

#ifdef CONFIG_AMLOGIC_BC
void amlogic_bc_init(void);
#else
static inline void amlogic_bc_init(void)
{
}
#endif

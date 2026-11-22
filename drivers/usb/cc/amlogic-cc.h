/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#include <linux/workqueue.h>

struct cc_regs {
	void __iomem	 *cc_r[6];
};

union cc_r0 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned cc_en:1;
		unsigned cc_sw_func_en:1;
		unsigned cc_usb2_sel:1;
		unsigned reserved1:1;
		unsigned cc_current_int_mask:1;
		unsigned cc_plugout_int_mask:1;
		unsigned cc_plugin_int_mask:1;
		unsigned cc_data_int_mask:1;
		unsigned cc_det_force_en:1;
		unsigned reserved2:3;
		unsigned cc_vbus_force_en:1;
		unsigned cc_filter_en:1;
		unsigned vbus_filter_en:1;
		unsigned reserved3:1;
		unsigned cc_vbus_det_sel_o:3;
		unsigned cc_ref_sel_o:1;
		unsigned cc1_rd_sel_o:1;
		unsigned cc2_rd_sel_o:1;
		unsigned reserved4:2;
		unsigned cc_error_list_mask:8;
	} b;
};

union cc_r1 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned cc_int_clr:1;
		unsigned reserved:31;
	} b;
};

union cc_r2 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned cc_int_status:4;
		unsigned reserved1:4;
		unsigned cc_power_level:2;
		unsigned reserved2:6;
		unsigned cc_plug_ab:2;
		unsigned reserved3:14;
	} b;
};

union cc_r3 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned cc_attach_force:1;
		unsigned cc_rp_out:1;
		unsigned cc_vbus_out:1;
		unsigned cc_usb2_work:1;
		unsigned cc1_ufp_det_o_d2:3;
		unsigned cc_vbusless_d2:1;
		unsigned cc2_ufp_det_o_d2:3;
		unsigned cc_vbus_ok_d2:1;
		unsigned cc_error_value:8;
		unsigned cc_state:3;
		unsigned cc_top_enable:1;
		unsigned cc_reg_pin_state1:3;
		unsigned reserved1:1;
		unsigned cc_reg_pin_state2:3;
		unsigned reserved2:1;
	} b;
};

union cc_r4 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned cc_otp_bg_trim:3;
		unsigned cc_otp_res_std_trim:4;
		unsigned cc_otp_func_en:1;
		unsigned reserved:24;
	} b;
};

union cc_r5 {
	/** raw register data */
	u32 d32;
	/** register bits */
	struct {
		unsigned tcc_cnt_num:8;
		unsigned trp_cnt_num:5;
		unsigned filter_cnt_num:3;
		unsigned tpd_cnt_num:5;
		unsigned reserved:11;
	} b;
};

struct cc_dev {
	struct device	*dev;
	void __iomem	*regs;
	void __iomem	*phy_base;
	struct delayed_work	work;
	u32 cc_status;
};

#ifdef CONFIG_AMLOGIC_CC
void amlogic_cc_init(void);
#else
static void amlogic_cc_init(void)
{
}
#endif

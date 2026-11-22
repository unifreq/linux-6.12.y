// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/clk-provider.h>
#include <linux/init.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/clkdev.h>
#include "clk-pll.h"
#include "clk-regmap.h"
#include "clk-dualdiv.h"
#include "gxlx4.h"
#include <dt-bindings/clock/gxlx4-clkc.h>

static const struct clk_ops meson_pll_clk_no_ops = {};

static const struct pll_params_table gxlx4_fix_pll_params_table[] = {
	PLL_PARAMS_OD(500, 3, 1), /*DCO=400M OD=DCO/2=2000M*/
	{ /* sentinel */ }
};

static struct clk_regmap gxlx4_fixed_pll = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift   = 11,
			.width   = 5,
		},
		.l = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.od = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift	 = 9,
			.width	 = 2,
		},
		.rst = {
			.reg_off = ANACTRL_FIXPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.table = gxlx4_fix_pll_params_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fixed_pll",
		.ops = &meson_clk_pll_ro_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div2_div = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div2_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fixed_pll.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div2 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div2",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div2_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div3_div = {
	.mult = 1,
	.div = 3,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div3_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fixed_pll.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div3 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 26,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div3",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div3_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div4_div = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div4_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fixed_pll.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div4 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 27,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div4",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div4_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div5_div = {
	.mult = 1,
	.div = 5,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div5_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fixed_pll.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div5 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 28,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div5",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div5_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div7_div = {
	.mult = 1,
	.div = 7,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div7_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fixed_pll.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div7 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 29,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div7",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div7_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_fclk_div2p5_div = {
	.mult = 2,
	.div = 5,
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div2p5_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fixed_pll.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_fclk_div2p5 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL0,
		.bit_idx = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "fclk_div2p5",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div2p5_div.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_mpll_50m_div = {
	.mult = 1,
	.div = 40,
	.hw.init = &(struct clk_init_data){
		.name = "mpll_50m_div",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fixed_pll.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_mpll_50m = {
	.data = &(struct clk_regmap_gate_data){
		.offset = ANACTRL_FIXPLL_CTRL1,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mpll_50m",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_mpll_50m_div.hw
		},
		.num_parents = 1,
	},
};

static const struct pll_params_table gxlx4_gp0_pll_table[] = {
	PLL_PARAMS_OD(256, 1, 1), /* DCO = 3072M OD = 2 PLL = 1536M */
	PLL_PARAMS_OD(192, 1, 1), /* DCO = 2304M OD = 1 PLL = 1152M */
	{ /* sentinel */  }
};

static const struct reg_sequence gxlx4_gp0_init_regs[] = {
	{ .reg = ANACTRL_GP0PLL_CTRL1,	.def = 0x09900322 },
	{ .reg = ANACTRL_GP0PLL_CTRL2,	.def = 0x0 }, /* frac = 0 as default */
	{ .reg = ANACTRL_GP0PLL_CTRL3,	.def = 0x6 },
};

static struct clk_regmap gxlx4_gp0_pll = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.n = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 11,
			.width   = 5,
		},
		.od = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift	 = 9,
			.width	 = 2,
		},
		.l = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = ANACTRL_GP0PLL_CTRL0,
			.shift   = 16,
			.width   = 1,
		},
		.table = gxlx4_gp0_pll_table,
		.init_regs = gxlx4_gp0_init_regs,
		.init_count = ARRAY_SIZE(gxlx4_gp0_init_regs),
		.flags = CLK_MESON_PLL_FIXED_EN0P5 | CLK_MESON_PLL_NOINIT_ENABLED,
	},
	.hw.init = &(struct clk_init_data){
		.name = "gp0_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct reg_sequence gxlx4_hifi_init_regs[] = {
	{ .reg = ANACTRL_HIFIPLL_CTRL0, .def = 0x02520000 },
	{ .reg = ANACTRL_HIFIPLL_CTRL1,	.def = 0x09900322 },
	{ .reg = ANACTRL_HIFIPLL_CTRL2,	.def = 0x00004e20 },
	{ .reg = ANACTRL_HIFIPLL_CTRL3,	.def = 0x6 },
};

static const struct pll_params_table hifi_pll_table[] = {
	PLL_PARAMS_OD(150, 1, 0), /* DCO = 1800M OD = 0 PLL = 1800M */
	PLL_PARAMS_OD(150, 1, 2), /* DCO = 1800M OD = 4 PLL = 450M */
	PLL_PARAMS_OD(163, 1, 2), /* DCO = 1944M OD = 4 PLL = 486M */
	{ /* sentinel */  }
};

static struct clk_regmap gxlx4_hifi_pll = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.n = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 11,
			.width   = 5,
		},
		.m = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.od = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift	 = 9,
			.width	 = 2,
		},
		.frac = {
			.reg_off = ANACTRL_HIFIPLL_CTRL2,
			.shift   = 0,
			.width   = 19,
		},
		.l = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = ANACTRL_HIFIPLL_CTRL0,
			.shift   = 16,
			.width   = 1,
		},
		.table = hifi_pll_table,
		.init_regs = gxlx4_hifi_init_regs,
		.init_count = ARRAY_SIZE(gxlx4_hifi_init_regs),
		.flags = CLK_MESON_PLL_FIXED_FRAC_WEIGHT_PRECISION |
			CLK_MESON_PLL_FIXED_EN0P5
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hifi_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct reg_sequence gxlx4_hifi1_init_regs[] = {
	{ .reg = ANACTRL_HIFI1PLL_CTRL0,	.def = 0x02520000 },
	{ .reg = ANACTRL_HIFI1PLL_CTRL1,	.def = 0x09900322 },
	{ .reg = ANACTRL_HIFI1PLL_CTRL2,	.def = 0x00004e20 },
	{ .reg = ANACTRL_HIFI1PLL_CTRL3,	.def = 0x6 },
};

static struct clk_regmap gxlx4_hifi1_pll = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.n = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 11,
			.width   = 5,
		},
		.m = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 0,
			.width   = 9,
		},
		.od = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift	 = 9,
			.width	 = 2,
		},
		.frac = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL2,
			.shift   = 0,
			.width   = 19,
		},
		.l = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.l_rst = {
			.reg_off = ANACTRL_HIFI1PLL_CTRL0,
			.shift   = 16,
			.width   = 1,
		},
		.table = hifi_pll_table,
		.init_regs = gxlx4_hifi1_init_regs,
		.init_count = ARRAY_SIZE(gxlx4_hifi1_init_regs),
		.flags = CLK_MESON_PLL_FIXED_FRAC_WEIGHT_PRECISION |
			CLK_MESON_PLL_FIXED_EN0P5
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hifi1_pll",
		.ops = &meson_clk_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static const struct reg_sequence gxlx4_usb_pll_init_regs[] = {
	{ .reg = ANACTRL_USBPLL_CTRL0,	.def = 0x200c0cc8 },
	{ .reg = ANACTRL_USBPLL_CTRL1,	.def = 0x0632c800 },
	{ .reg = ANACTRL_USBPLL_CTRL0,	.def = 0x300c0cc8, .delay_us = 20 },
	{ .reg = ANACTRL_USBPLL_CTRL0,	.def = 0x100c0cc8, .delay_us = 20 },
	{ .reg = ANACTRL_USBPLL_CTRL1,	.def = 0x06328800 },
};

static const struct pll_params_table gxlx4_usb_pll_table[] = {
	PLL_PARAMS_OD(200, 2, 0),
	{0, 0, 0}
};

static struct clk_regmap gxlx4_usb_pll_dco = {
	.data = &(struct meson_clk_pll_data){
		.en = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.m = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift   = 0,
			.width   = 8,
		},
		.n = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift   = 11,
			.width   = 5,
		},
		.od = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift	 = 9,
			.width	 = 2,
		},
		.l = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift   = 31,
			.width   = 1,
		},
		.rst = {
			.reg_off = ANACTRL_USBPLL_CTRL0,
			.shift   = 29,
			.width   = 1,
		},
		.table = gxlx4_usb_pll_table,
		.init_regs = gxlx4_usb_pll_init_regs,
		.init_count = ARRAY_SIZE(gxlx4_usb_pll_init_regs),
	},
	.hw.init = &(struct clk_init_data){
		.name = "usb_pll_dco",
		.ops = &meson_clk_pcie_pll_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_fixed_factor gxlx4_usb_pll = {
	.mult = 1,
	.div = 25,
	.hw.init = &(struct clk_init_data){
		.name = "usb_pll",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_usb_pll_dco.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_rtc_32k_in = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_32k_in",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param clk_32k_div_table[] = {
	{
		.n1	= 733, .m1	= 8,
		.n2	= 732, .m2	= 11,
		.dual	= 1,
	},
	{}
};

static struct clk_regmap gxlx4_rtc_32k_div = {
	.data = &(struct meson_clk_dualdiv_data){
		.n1 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_RTC_BY_OSCIN_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.table = clk_32k_div_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "rtc_32k_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_rtc_32k_in.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_rtc_32k_force_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL1,
		.mask = 0x1,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "rtc_32k_force_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_rtc_32k_div.hw,
			&gxlx4_rtc_32k_in.hw,
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_rtc_32k_out = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_RTC_BY_OSCIN_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "rtc_32k_out",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_rtc_32k_force_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_rtc_32k_mux0_0 = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_CTRL,
		.mask = 0x1,
		.shift = 0,
	},
	.hw.init = &(struct clk_init_data){
		.name = "rtc_32k_mux0_0",
		.ops = &clk_regmap_mux_ops,
		.parent_data = (const struct clk_parent_data []) {
			{ .fw_name = "xtal", },
			{ .hw = &gxlx4_rtc_32k_out.hw },
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_rtc_32k_mux0_1 = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_CTRL,
		.mask = 0x1,
		.shift = 0,
	},
	.hw.init = &(struct clk_init_data){
		.name = "rtc_32k_mux0_1",
		.ops = &clk_regmap_mux_ops,
		.parent_data = (const struct clk_parent_data []) {
			{ .fw_name = "pad", },
			{ .fw_name = "xtal", },
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_rtc_clk = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_RTC_CTRL,
		.mask = 0x1,
		.shift = 1,
	},
	.hw.init = &(struct clk_init_data){
		.name = "rtc",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_rtc_32k_mux0_0.hw,
			&gxlx4_rtc_32k_mux0_1.hw,
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 mux_table_sys_clk_sel[] = { 0, 1, 2, 3 };

static const struct clk_parent_data gxlx4_sys_clk_sel[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div2.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div4.hw }
};

static struct clk_regmap gxlx4_sys_clk_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.mask = 0x7,
		.shift = 10,
		.table = mux_table_sys_clk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sys_clk_0_sel",
		.ops = &clk_regmap_mux_ro_ops,
		.parent_data = gxlx4_sys_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_sys_clk_sel),
	},
};

static struct clk_regmap gxlx4_sys_clk_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.shift = 0,
		.width = 10,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sys_clk_0_div",
		.ops = &clk_regmap_divider_ro_ops,
		.parent_names = (const char *[]){ "sys_clk_0_sel" },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_sys_clk_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.bit_idx = 13,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sys_clk_0",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_names = (const char *[]){ "sys_clk_0_div" },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_sys_clk_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.mask = 0x7,
		.shift = 26,
		.table = mux_table_sys_clk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sys_clk_1_sel",
		.ops = &clk_regmap_mux_ro_ops,
		.parent_data = gxlx4_sys_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_sys_clk_sel),
	},
};

static struct clk_regmap gxlx4_sys_clk_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.shift = 16,
		.width = 10,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sys_clk_1_div",
		.ops = &clk_regmap_divider_ro_ops,
		.parent_names = (const char *[]){ "sys_clk_1_sel" },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_sys_clk_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.bit_idx = 29,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sys_clk_1",
		.ops = &clk_regmap_gate_ro_ops,
		.parent_names = (const char *[]){ "sys_clk_1_div" },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_sys_clk = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SYS_CLK_CTRL0,
		.mask = 0x1,
		.shift = 15,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sys_clk",
		.ops = &clk_regmap_mux_ro_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_sys_clk_0.hw,
			&gxlx4_sys_clk_1.hw
		},
		.num_parents = 2,
	},
};

static u32 mux_table_axi_clk_sel[] = { 0, 1, 2, 3, 4, 6, 7 };

static const struct clk_parent_data gxlx4_axi_clk_sel[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div2.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div2p5.hw },
	{ .hw = &gxlx4_rtc_clk.hw }
};

static struct clk_regmap gxlx4_axi_clk_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.mask = 0x7,
		.shift = 10,
		.table = mux_table_axi_clk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "axi_clk_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_axi_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_axi_clk_sel),
	},
};

static struct clk_regmap gxlx4_axi_clk_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.shift = 0,
		.width = 10,
	},
	.hw.init = &(struct clk_init_data){
		.name = "axi_clk_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_names = (const char *[]){ "axi_clk_0_sel" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_axi_clk_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.bit_idx = 13,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "axi_clk_0",
		.ops = &clk_regmap_gate_ops,
		.parent_names = (const char *[]){ "axi_clk_0_div" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
	},
};

static struct clk_regmap gxlx4_axi_clk_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.mask = 0x7,
		.shift = 26,
		.table = mux_table_axi_clk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "axi_clk_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_axi_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_axi_clk_sel),
	},
};

static struct clk_regmap gxlx4_axi_clk_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.shift = 16,
		.width = 10,
	},
	.hw.init = &(struct clk_init_data){
		.name = "axi_clk_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_names = (const char *[]){ "axi_clk_1_sel" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_axi_clk_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.bit_idx = 29,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "axi_clk_1",
		.ops = &clk_regmap_gate_ops,
		.parent_names = (const char *[]){ "axi_clk_1_div" },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IS_CRITICAL,
	},
};

static struct clk_regmap gxlx4_axi_clk = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AXI_CLK_CTRL0,
		.mask = 0x1,
		.shift = 15,
	},
	.hw.init = &(struct clk_init_data){
		.name = "axi_clk",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_axi_clk_0.hw,
			&gxlx4_axi_clk_1.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_saradc_mux_table[] = { 0, 1, 2 };

static const struct clk_parent_data gxlx4_saradc_sel_clk_sel[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_sys_clk.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
};

static struct clk_regmap gxlx4_saradc_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_saradc_mux_table
	},
	.hw.init = &(struct clk_init_data){
		.name = "saradc_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_saradc_sel_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_saradc_sel_clk_sel),
	},
};

static struct clk_regmap gxlx4_saradc_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "saradc_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_saradc_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_saradc = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_SAR_CLK_CTRL0,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "saradc",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_saradc_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cecb_32k_clkin = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CECB_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cecb_32k_clkin",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_cecb_32k_div = {
	.data = &(struct meson_clk_dualdiv_data){
		.n1 = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_CECB_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_CECB_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_CECB_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.table = clk_32k_div_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cecb_32k_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cecb_32k_clkin.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_cecb_32k_sel_pre = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECB_CTRL1,
		.mask = 0x1,
		.shift = 24,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cecb_32k_sel_pre",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cecb_32k_div.hw,
			&gxlx4_cecb_32k_clkin.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cecb_32k_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_CECB_CTRL1,
		.mask = 0x1,
		.shift = 31,
		.flags = CLK_MUX_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cecb_32k_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cecb_32k_sel_pre.hw,
			&gxlx4_rtc_clk.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cecb_32k_clkout = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CECB_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cecb_32k_clkout",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cecb_32k_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_sc_mux_table[] = {0, 1, 2, 3, 4};

static const struct clk_parent_data gxlx4_sc_parent_data[] = {
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div2.hw },
};

static struct clk_regmap gxlx4_sc_clk_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = gxlx4_sc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sc_clk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_sc_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_sc_parent_data),
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_sc_clk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_SC_CLK_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sc_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_sc_clk_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_sc_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_SC_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sc_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_sc_clk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static u32 mux_table_vclk_sel[] = {1, 2, 3, 4, 5, 6, 7};
static const struct clk_hw *gxlx4_vclk_parent_hws[] = {
	//&gxlx4_vid_pll.hw, //TODO: Need to confirm vid pll with vlsi
	&gxlx4_gp0_pll.hw,
	&gxlx4_hifi_pll.hw,
	&gxlx4_hifi1_pll.hw,
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw
};

static struct clk_regmap gxlx4_vclk_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.mask = 0x7,
		.shift = 16,
		.table = mux_table_vclk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vclk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vclk_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vclk_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vclk2_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.mask = 0x7,
		.shift = 16,
		.table = mux_table_vclk_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vclk_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vclk_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vclk_input = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_DIV,
		.bit_idx = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_input",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_input = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.bit_idx = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_input",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VID_CLK0_DIV,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vclk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk_input.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vclk2_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk2_input.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vclk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 19,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 19,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_div1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div2_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 1,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_div2_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div4_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_div4_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div6_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_div6_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk_div12_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk_div12_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_div1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_div1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_div2_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 1,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_div2_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_div4_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_div4_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_div6_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 3,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_div6_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vclk2_div12_en = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VIID_CLK0_CTRL,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vclk2_div12_en",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vclk2.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_fixed_factor gxlx4_vclk_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data){
		.name = "vclk_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk_div2_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk_div4 = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data){
		.name = "vclk_div4",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk_div4_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk_div6 = {
	.mult = 1,
	.div = 6,
	.hw.init = &(struct clk_init_data){
		.name = "vclk_div6",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk_div6_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk_div12 = {
	.mult = 1,
	.div = 12,
	.hw.init = &(struct clk_init_data){
		.name = "vclk_div12",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk_div12_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk2_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk2_div2_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk2_div4 = {
	.mult = 1,
	.div = 4,
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_div4",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk2_div4_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk2_div6 = {
	.mult = 1,
	.div = 6,
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_div6",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk2_div6_en.hw
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_vclk2_div12 = {
	.mult = 1,
	.div = 12,
	.hw.init = &(struct clk_init_data){
		.name = "vclk2_div12",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vclk2_div12_en.hw
		},
		.num_parents = 1,
	},
};

static u32 mux_table_cts_sel[] = { 0, 1, 2, 3, 4, 8, 9, 10, 11, 12 };
static const struct clk_hw *gxlx4_cts_parent_hws[] = {
	&gxlx4_vclk_div1.hw,
	&gxlx4_vclk_div2.hw,
	&gxlx4_vclk_div4.hw,
	&gxlx4_vclk_div6.hw,
	&gxlx4_vclk_div12.hw,
	&gxlx4_vclk2_div1.hw,
	&gxlx4_vclk2_div2.hw,
	&gxlx4_vclk2_div4.hw,
	&gxlx4_vclk2_div6.hw,
	&gxlx4_vclk2_div12.hw
};

static struct clk_regmap gxlx4_cts_enci_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VID_CLK0_DIV,
		.mask = 0xf,
		.shift = 28,
		.table = mux_table_cts_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cts_enci_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_cts_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_cts_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_cts_vdac_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VIID_CLK0_DIV,
		.mask = 0xf,
		.shift = 28,
		.table = mux_table_cts_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cts_vdac_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_cts_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_cts_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_cts_encp_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VID_CLK0_DIV,
		.mask = 0xf,
		.shift = 20,
		.table = mux_table_cts_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cts_encp_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_cts_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_cts_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

/* TOFIX: add support for cts_tcon */
static u32 mux_table_hdmi_tx_sel[] = { 0, 1, 2, 3, 4, 8, 9, 10, 11, 12 };
static const struct clk_hw *gxlx4_cts_hdmi_tx_parent_hws[] = {
	&gxlx4_vclk_div1.hw,
	&gxlx4_vclk_div2.hw,
	&gxlx4_vclk_div4.hw,
	&gxlx4_vclk_div6.hw,
	&gxlx4_vclk_div12.hw,
	&gxlx4_vclk2_div1.hw,
	&gxlx4_vclk2_div2.hw,
	&gxlx4_vclk2_div4.hw,
	&gxlx4_vclk2_div6.hw,
	&gxlx4_vclk2_div12.hw
};

static struct clk_regmap gxlx4_hdmi_tx_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HDMI_CLK_CTRL,
		.mask = 0xf,
		.shift = 16,
		.table = mux_table_hdmi_tx_sel,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmi_tx_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_cts_hdmi_tx_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_cts_hdmi_tx_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_cts_enci = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 0,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_enci",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_enci_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_cts_encp = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 2,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_encp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_encp_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_cts_vdac = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_vdac",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_vdac_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_hdmi_tx = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_CLK0_CTRL2,
		.bit_idx = 5,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmi_tx",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hdmi_tx_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

/*mali_clk*/
/*
 * The MALI IP is clocked by two identical clocks (mali_0 and mali_1)
 * muxed by a glitch-free switch on Meson8b and Meson8m2 and later.
 *
 * CLK_SET_RATE_PARENT is added for mali_0_sel clock
 * 1.gp0 pll only support the 846M, avoid other rate 500/400M from it
 * 2.hifi pll is used for other module, skip it, avoid some rate from it
 */
static u32 mux_table_mali[] = { 0, 1, 3, 4, 5, 6};

static const struct clk_parent_data gxlx4_mali_0_1_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_gp0_pll.hw },
	{ .hw = &gxlx4_fclk_div2p5.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
};

static struct clk_regmap gxlx4_mali_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = mux_table_mali,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_mali_0_1_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_mali_0_1_parent_data),
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_mali_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_mali_0_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_mali_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_mali_0_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_mali_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = mux_table_mali,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_mali_0_1_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_mali_0_1_parent_data),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_mali_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_mali_1_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_mali_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_mali_1_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT,
	},
};

static const struct clk_hw *gxlx4_mali_parent_hws[] = {
	&gxlx4_mali_0.hw,
	&gxlx4_mali_1.hw
};

static struct clk_regmap gxlx4_mali = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_MALI_CLK_CTRL,
		.mask = 1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data){
		.name = "mali",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_mali_parent_hws,
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 mux_table_vdec[] = { 0, 1, 2, 3, 4, 6};

static const struct clk_hw *gxlx4_vdec_parent_hws[] = {
	&gxlx4_fclk_div2p5.hw,
	&gxlx4_fclk_div3.hw,
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw,
	&gxlx4_gp0_pll.hw
};

static struct clk_regmap gxlx4_hevcb_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
		.flags = CLK_MUX_ROUND_CLOSEST,
		.table = mux_table_vdec,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hevcb_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vdec_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vdec_parent_hws),
	},
};

static struct clk_regmap gxlx4_hevcb_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.shift = 16,
		.width = 7,
		.flags = CLK_DIVIDER_ROUND_CLOSEST,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hevcb_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hevcb_0_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_hevcb_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VDEC2_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevcb_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hevcb_0_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_hevcb_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevcb_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vdec_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vdec_parent_hws),
	},
};

static struct clk_regmap gxlx4_hevcb_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevc_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hevcb_1_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_hevcb_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hevcb_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hevcb_1_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_hevcb = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VDEC4_CLK_CTRL,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hevcb",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_hevcb_0.hw,
			&gxlx4_hevcb_1.hw
		},
		.num_parents = 2,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static const struct clk_hw *gxlx4_vpu_parent_hws[] = {
	&gxlx4_fclk_div3.hw,
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw
};

static struct clk_regmap gxlx4_vpu_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vpu_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vpu_parent_hws),
	},
};

static struct clk_regmap gxlx4_vpu_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vpu_0_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vpu_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vpu_0_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vpu_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vpu_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vpu_parent_hws),
	},
};

static struct clk_regmap gxlx4_vpu_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vpu_1_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vpu_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vpu_1_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vpu = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VPU_CLK_CTRL,
		.mask = 1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu",
		.ops = &clk_regmap_mux_ops,
		/*
		 * bit 31 selects from 2 possible parents:
		 * vpu_0 or vpu_1
		 */
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vpu_0.hw,
			&gxlx4_vpu_1.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_NO_REPARENT,
	},
};

static const struct clk_hw *vpu_clkb_tmp_parent_hws[] = {
	&gxlx4_vpu.hw,
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw
};

static struct clk_regmap gxlx4_vpu_clkb_tmp_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.mask = 0x3,
		.shift = 20,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_tmp_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = vpu_clkb_tmp_parent_hws,
		.num_parents = ARRAY_SIZE(vpu_clkb_tmp_parent_hws),
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vpu_clkb_tmp_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.shift = 16,
		.width = 4,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_tmp_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vpu_clkb_tmp_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vpu_clkb_tmp = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_clkb_tmp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vpu_clkb_tmp_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vpu_clkb_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vpu_clkb_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vpu_clkb_tmp.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vpu_clkb = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VPU_CLKB_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vpu_clkb",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vpu_clkb_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_vapb_table[] = { 0, 1, 2, 3, 7};
static const struct clk_hw *gxlx4_vapb_parent_hws[] = {
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div3.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw,
	&gxlx4_fclk_div2p5.hw
};

static struct clk_regmap gxlx4_vapb_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = gxlx4_vapb_table
	},
	.hw.init = &(struct clk_init_data){
		.name = "vapb_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vapb_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vapb_parent_hws),
	},
};

static struct clk_regmap gxlx4_vapb_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vapb_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vapb_0_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vapb_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vapb_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vapb_0_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vapb_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.mask = 0x7,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vapb_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_vapb_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vapb_parent_hws),
		.flags = CLK_SET_RATE_NO_REPARENT,
	},
};

static struct clk_regmap gxlx4_vapb_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vapb_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vapb_1_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vapb_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vapb_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vapb_1_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_vapb = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.mask = 1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vapb_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vapb_0.hw,
			&gxlx4_vapb_1.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_ge2d_gate = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VAPBCLK_CTRL,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "ge2d_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vapb.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_parent_data gxlx4_hdmirx_sys_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div5.hw }
};

static struct clk_regmap gxlx4_hdmirx_5m_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_5m_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_5m_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_5m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_5m_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_5m  = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_5m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_5m_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_2m_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_2m_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_2m_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_2m_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_2m_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_2m = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL0,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_2m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_2m_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_cfg_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_cfg_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_cfg_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_cfg_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_cfg_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_cfg  = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_cfg",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_cfg_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_hdcp_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_hdcp_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_hdcp_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_hdcp_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_hdcp_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_hdcp = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL1,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_hdcp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_hdcp_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_aud_pll_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_aud_pll_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_aud_pll_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_aud_pll_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_aud_pll_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_aud_pll  = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_aud_pll",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_aud_pll_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_acr_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_acr_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_acr_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_acr_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_acr_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_acr = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL2,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_acr",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_acr_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_meter_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_HRX_CLK_CTRL3,
		.mask = 0x3,
		.shift = 9,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_meter_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_hdmirx_sys_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_hdmirx_sys_parent_data),
	},
};

static struct clk_regmap gxlx4_hdmirx_meter_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_HRX_CLK_CTRL3,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "hdmirx_meter_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_meter_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_hdmirx_meter  = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_HRX_CLK_CTRL3,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "hdmirx_meter",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_hdmirx_meter_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static u32 gxlx4_pwm_clk_table[] = {0, 2, 3};

static const struct clk_parent_data gxlx4_pwm_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw }
};

static struct clk_regmap gxlx4_pwm_a_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_a_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_a_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_a_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_a_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_a = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_a",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_a_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_b_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.mask = 0x3,
		.shift = 25,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_b_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_b_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_b_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_b_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_b = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_AB_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_b",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_b_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_c_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_c_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_c_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_c_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_c_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_c = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_c",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_c_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_d_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.mask = 0x3,
		.shift = 25,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_d_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_d_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_d_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_d_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_d = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_CD_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_d",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_d_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_e_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_e_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_e_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_e_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_e_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_e = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_e",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_e_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_f_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.mask = 0x3,
		.shift = 25,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_f_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_f_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_f_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_f_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_f = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_EF_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_f",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_f_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_g_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_g_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_g_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_g_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_g_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_g = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_g",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_g_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_pwm_h_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.mask = 0x3,
		.shift = 25,
		.table = gxlx4_pwm_clk_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_h_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_pwm_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_pwm_parent_data),
	},
};

static struct clk_regmap gxlx4_pwm_h_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.shift = 16,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_h_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_h_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_pwm_h = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_PWM_CLK_GH_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "pwm_h",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_pwm_h_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static u32 gxlx4_spicc_mux_table[] = {0, 1, 2, 3, 4, 5, 6};

static const struct clk_parent_data gxlx4_spicc_parent_hws[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_sys_clk.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div2.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div7.hw },
};

static struct clk_regmap gxlx4_spicc0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.mask = 0x7,
		.shift = 7,
		.table = gxlx4_spicc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_spicc_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_spicc_parent_hws),
	},
};

static struct clk_regmap gxlx4_spicc0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.shift = 0,
		.width = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_spicc0_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_spicc0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.bit_idx = 6,
	},
	.hw.init = &(struct clk_init_data){
		.name = "spicc0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_spicc0_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_spicc1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.mask = 0x7,
		.shift = 23,
		.table = gxlx4_spicc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_spicc_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_spicc_parent_hws),
	},
};

static struct clk_regmap gxlx4_spicc1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.shift = 16,
		.width = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "spicc1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_spicc1_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_spicc1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_SPICC_CLK_CTRL,
		.bit_idx = 22,
	},
	.hw.init = &(struct clk_init_data){
		.name = "spicc1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_spicc1_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_sd_emmc_mux_table[] = {0, 1, 2, 4, 7};

static const struct clk_parent_data gxlx4_sd_emmc_parent_data[]  = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div2.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div2p5.hw },
	{ .hw = &gxlx4_gp0_pll.hw }
};

static struct clk_regmap gxlx4_sd_emmc_c_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = gxlx4_sd_emmc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_sd_emmc_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_sd_emmc_parent_data),
		.flags = CLK_GET_RATE_NOCACHE
	},
};

static struct clk_regmap gxlx4_sd_emmc_c_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "sd_emmc_c_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_sd_emmc_c_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE //| CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_sd_emmc_c = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_NAND_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "sd_emmc_c",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_sd_emmc_c_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE //| CLK_SET_RATE_PARENT
	},
};

static const struct clk_parent_data gxlx4_vdin_parent_hws[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
};

static u32 gxlx4_vdin_meas_table[] = {0, 1, 2, 3};

static struct clk_regmap gxlx4_vdin_meas_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = gxlx4_vdin_meas_table,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdin_meas_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_vdin_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_vdin_parent_hws),
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_vdin_meas_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vdin_meas_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vdin_meas_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vdin_meas = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VDIN_MEAS_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vdin_meas",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_vdin_meas_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_vid_lock_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_VID_LOCK_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "vid_lock_div",
		.ops = &clk_regmap_divider_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_vid_lock_clk  = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_VID_LOCK_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "vid_lock_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_vid_lock_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static u32 gxlx4_eth_rmii_table[] = { 0 };

static struct clk_regmap gxlx4_eth_rmii_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_eth_rmii_table
	},
	.hw.init = &(struct clk_init_data){
		.name = "eth_rmii_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div2.hw,
		},
		.num_parents = 1
	},
};

static struct clk_regmap gxlx4_eth_rmii_div = {
	.data = &(struct clk_regmap_div_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "eth_rmii_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_eth_rmii_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_eth_rmii = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "eth_rmii",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_eth_rmii_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_fixed_factor gxlx4_eth_div8 = {
	.mult = 1,
	.div = 8,
	.hw.init = &(struct clk_init_data){
		.name = "eth_div8",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_fclk_div2.hw },
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_eth_125m = {
	.data = &(struct clk_regmap_gate_data) {
		.offset = CLKCTRL_ETH_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "eth_125m",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_eth_div8.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_ts_clk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_TS_CLK_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "ts_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_ts_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_TS_CLK_CTRL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "ts_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_ts_clk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static u32 gxlx4_tcon_pll_mux_table[] = { 0, 1, 2, 3 };

static const struct clk_parent_data gxlx4_cts_tcon_pll_clk_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
};

static struct clk_regmap gxlx4_cts_tcon_pll_clk_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_TCON_CLK_CNTL,
		.mask = 0x7,
		.shift = 7,
		.table = gxlx4_tcon_pll_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_tcon_pll_clk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_cts_tcon_pll_clk_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_cts_tcon_pll_clk_parent_data),
	},
};

static struct clk_regmap gxlx4_cts_tcon_pll_clk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_TCON_CLK_CNTL,
		.shift = 0,
		.width = 6,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_tcon_pll_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_tcon_pll_clk_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cts_tcon_pll_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_TCON_CLK_CNTL,
		.bit_idx = 6,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cts_tcon_pll_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_tcon_pll_clk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_adc_extclk_mux_table[] = { 0, 1, 2, 3, 4 };

static const struct clk_parent_data gxlx4_adc_extclk_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div7.hw },
};

static struct clk_regmap gxlx4_adc_extclk_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.mask = 0x7,
		.shift = 25,
		.table = gxlx4_adc_extclk_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "adc_extclk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_adc_extclk_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_adc_extclk_parent_data),
	},
};

static struct clk_regmap gxlx4_adc_extclk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "adc_extclk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_adc_extclk_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_adc_extclk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "adc_extclk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_adc_extclk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_demod_32k_clkin = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_DEMOD_32K_CNTL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "demod_32k_clkin",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param gxlx4_demod_32k_div_table[] = {
	{
		.dual	= 0,
		.n1	= 733,
	},
	{}
};

static struct clk_regmap gxlx4_demod_32k_div = {
	.data = &(struct meson_clk_dualdiv_data){
		.n1 = {
			.reg_off = CLKCTRL_DEMOD_32K_CNTL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_DEMOD_32K_CNTL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_DEMOD_32K_CNTL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_DEMOD_32K_CNTL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_DEMOD_32K_CNTL0,
			.shift   = 28,
			.width   = 1,
		},
		.table = gxlx4_demod_32k_div_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "demod_32k_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_demod_32k_clkin.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_demod_32k = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_DEMOD_32K_CNTL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data){
		.name = "demod_32k",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_demod_32k_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_demod_core_mux_table[] = { 0, 1, 2 };

static const struct clk_parent_data gxlx4_cts_demod_core_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div7.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
};

static struct clk_regmap gxlx4_cts_demod_core_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_demod_core_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_demod_core_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_cts_demod_core_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_cts_demod_core_parent_data),
	},
};

static struct clk_regmap gxlx4_cts_demod_core_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cts_demod_core_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_demod_core_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cts_demod_core = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cts_demod_core",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_demod_core_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_demod_core_t2_mux_table[] = { 0, 1, 2 };

static const struct clk_parent_data gxlx4_cts_demod_core_t2_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
};

static struct clk_regmap gxlx4_cts_demod_core_t2_clk_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL1,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_demod_core_t2_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "demod_core_t2_clk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_cts_demod_core_t2_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_cts_demod_core_t2_parent_data),
	},
};

static struct clk_regmap gxlx4_cts_demod_core_t2_clk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL1,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "demod_core_t2_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_demod_core_t2_clk_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_cts_demod_core_t2_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_DEMOD_CLK_CNTL1,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "demod_core_t2_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cts_demod_core_t2_clk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_cdac_mux_table[] = {0, 1};

static const struct clk_parent_data gxlx4_cdac_parent_data[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_fclk_div5.hw },
};

static struct clk_regmap gxlx4_cdac_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.mask = 0x3,
		.shift = 16,
		.table = gxlx4_cdac_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cdac_clk_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_cdac_parent_data,
		.num_parents = ARRAY_SIZE(gxlx4_cdac_parent_data),
		.flags = CLK_GET_RATE_NOCACHE
	},
};

static struct clk_regmap gxlx4_cdac_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.shift = 0,
		.width = 16,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "cdac_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cdac_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_cdac = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CDAC_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "cdac",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_cdac_div.hw
		},
		.num_parents = 1,
		.flags = CLK_GET_RATE_NOCACHE |	CLK_SET_RATE_PARENT
	},
};

static u32 gxlx4_tsin_mux_table[] = { 0, 2, 3, 4, 5, 6 };

static const struct clk_hw *gxlx4_tsin_parent_hws[] = {
	&gxlx4_fclk_div2.hw,
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div3.hw,
	&gxlx4_fclk_div2p5.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw,
};

static struct clk_regmap gxlx4_tsin_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.mask = 0x3,
		.shift = 9,
		.table = gxlx4_tsin_mux_table
	},
	.hw.init = &(struct clk_init_data){
		.name = "tsin_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_tsin_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_tsin_parent_hws),
		.flags = CLK_GET_RATE_NOCACHE,
	},
};

static struct clk_regmap gxlx4_tsin_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "tsin_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_tsin_0_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_tsin_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.bit_idx = 8,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "tsin_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_tsin_0_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT
				 | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_tsin_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.mask = 0x3,
		.shift = 25,
	},
	.hw.init = &(struct clk_init_data){
		.name = "tsin_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_tsin_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_tsin_parent_hws),
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_tsin_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "tsin_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_tsin_1_sel.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_tsin_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "tsin_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) { &gxlx4_tsin_1_div.hw },
		.num_parents = 1,
		.flags = CLK_SET_RATE_GATE | CLK_SET_RATE_PARENT
				 | CLK_IGNORE_UNUSED,
	},
};

static struct clk_regmap gxlx4_tsin_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.mask = 0x1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "tsin_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_tsin_0.hw,
			&gxlx4_tsin_1.hw
		},
		.num_parents = 2,
		.flags = CLK_GET_RATE_NOCACHE | CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_tsin = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_TSIN_CLK_CNTL,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "tsin",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_tsin_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT | CLK_IGNORE_UNUSED,
	},
};

static u32 gxlx4_gen_clk_mux_table[] = { 0, 1, 5, 7, 8, 19, 20, 21, 22, 23, 24 };

static const struct clk_parent_data gxlx4_gen_sel_clk_sel[] = {
	{ .fw_name = "xtal", },
	{ .hw = &gxlx4_rtc_clk.hw },
	{ .hw = &gxlx4_gp0_pll.hw },
	{ .hw = &gxlx4_hifi_pll.hw },
	{ .hw = &gxlx4_hifi1_pll.hw },
	{ .hw = &gxlx4_fclk_div2.hw },
	{ .hw = &gxlx4_fclk_div2p5.hw },
	{ .hw = &gxlx4_fclk_div3.hw },
	{ .hw = &gxlx4_fclk_div4.hw },
	{ .hw = &gxlx4_fclk_div5.hw },
	{ .hw = &gxlx4_fclk_div7.hw },
};

static struct clk_regmap gxlx4_gen_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.mask = 0x1f,
		.shift = 12,
		.table = gxlx4_gen_clk_mux_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "gen_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_data = gxlx4_gen_sel_clk_sel,
		.num_parents = ARRAY_SIZE(gxlx4_gen_sel_clk_sel),
	},
};

static struct clk_regmap gxlx4_gen_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.shift = 0,
		.width = 11,
	},
	.hw.init = &(struct clk_init_data){
		.name = "gen_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_gen_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_gen = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_GEN_CLK_CTRL,
		.bit_idx = 11,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "gen",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_gen_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_24m_clk_gate = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CLK12_24_CTRL,
		.bit_idx = 11,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "24m_clk_gate",
		.ops = &clk_regmap_gate_ops,
		.parent_data = &(const struct clk_parent_data) {
			.fw_name = "xtal",
		},
		.num_parents = 1,
	},
};

static struct clk_fixed_factor gxlx4_24m_div2 = {
	.mult = 1,
	.div = 2,
	.hw.init = &(struct clk_init_data){
		.name = "24m_div2",
		.ops = &clk_fixed_factor_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_24m_clk_gate.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_12m_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CLK12_24_CTRL,
		.bit_idx = 10,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "12m_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_24m_div2.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_25m_clk_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_CLK12_24_CTRL,
		.shift = 0,
		.width = 8,
	},
	.hw.init = &(struct clk_init_data){
		.name = "25m_clk_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div2.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_25m_clk = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_CLK12_24_CTRL,
		.bit_idx = 12,
	},
	.hw.init = &(struct clk_init_data){
		.name = "25m_clk",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_25m_clk_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static u32 gxlx4_amfc_mux_table[] = {0, 1, 2, 3, 6};

static const struct clk_hw *gxlx4_amfc_parent_hws[] = {
	&gxlx4_fclk_div4.hw,
	&gxlx4_fclk_div3.hw,
	&gxlx4_fclk_div5.hw,
	&gxlx4_fclk_div7.hw,
	&gxlx4_gp0_pll.hw,
};

static struct clk_regmap gxlx4_amfc_0_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.mask = 0x7,
		.shift = 9,
		.table = gxlx4_amfc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "amfc_0_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_amfc_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_amfc_parent_hws),
	},
};

static struct clk_regmap gxlx4_amfc_0_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.shift = 0,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "amfc_0_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_amfc_0_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_amfc_0 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.bit_idx = 7,
	},
	.hw.init = &(struct clk_init_data){
		.name = "amfc_0",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_amfc_0_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_amfc_1_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.mask = 0x7,
		.shift = 25,
		.table = gxlx4_amfc_mux_table
	},
	.hw.init = &(struct clk_init_data) {
		.name = "amfc_1_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_amfc_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_amfc_parent_hws),
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_amfc_1_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.shift = 16,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "amfc_1_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_amfc_1_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_amfc_1 = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.bit_idx = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "amfc_1",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_amfc_1_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_amfc = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_AMFC_CLK_CTRL,
		.mask = 1,
		.shift = 31,
	},
	.hw.init = &(struct clk_init_data){
		.name = "amfc",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_amfc_0.hw,
			&gxlx4_amfc_1.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_usb2_48m_pre_in = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.bit_idx = 31,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb2_48m_pre_in",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_fclk_div2.hw
		},
		.num_parents = 1,
	},
};

static const struct meson_clk_dualdiv_param usb_48m_div_table[] = {
	{
		.n1		= 20, .m1		= 0,
		.n2		= 21, .m2		= 5,
		.dual		= 1,
	},
	{}
};

static struct clk_regmap gxlx4_usb2_48m_pre_div = {
	.data = &(struct meson_clk_dualdiv_data){
		.n1 = {
			.reg_off = CLKCTRL_USB_CLK_CTRL0,
			.shift   = 0,
			.width   = 12,
		},
		.n2 = {
			.reg_off = CLKCTRL_USB_CLK_CTRL0,
			.shift   = 12,
			.width   = 12,
		},
		.m1 = {
			.reg_off = CLKCTRL_USB_CLK_CTRL1,
			.shift   = 0,
			.width   = 12,
		},
		.m2 = {
			.reg_off = CLKCTRL_USB_CLK_CTRL1,
			.shift   = 12,
			.width   = 12,
		},
		.dual = {
			.reg_off = CLKCTRL_USB_CLK_CTRL0,
			.shift   = 28,
			.width   = 1,
		},
		.table = usb_48m_div_table,
	},
	.hw.init = &(struct clk_init_data){
		.name = "usb2_48m_pre_div",
		.ops = &meson_clk_dualdiv_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_pre_in.hw
		},
		.num_parents = 1,
	},
};

static struct clk_regmap gxlx4_usb2_48m_pre_force_sel = {
	.data = &(struct clk_regmap_mux_data) {
		.offset = CLKCTRL_USB_CLK_CTRL1,
		.mask = 0x1,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data){
		.name = "usb2_48m_pre_force_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_pre_div.hw,
			&gxlx4_usb2_48m_pre_in.hw,
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static struct clk_regmap gxlx4_usb2_48m_pre = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.bit_idx = 30,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb2_48m_pre",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_pre_force_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT,
	},
};

static const struct clk_hw *gxlx4_usb2_48m_clk_tmp_parent_hws[] = {
	&gxlx4_gp0_pll.hw,
	&gxlx4_usb_pll.hw,
	&gxlx4_hifi_pll.hw,
	&gxlx4_hifi1_pll.hw,
};

static struct clk_regmap gxlx4_usb2_48m_clk_tmp_sel = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.mask = 0x3,
		.shift = 24,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb2_48m_tmp_sel",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = gxlx4_usb2_48m_clk_tmp_parent_hws,
		.num_parents = ARRAY_SIZE(gxlx4_usb2_48m_clk_tmp_parent_hws),
	},
};

static struct clk_regmap gxlx4_usb2_48m_clk_tmp_div = {
	.data = &(struct clk_regmap_div_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.shift = 25,
		.width = 7,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb2_48m_tmp_div",
		.ops = &clk_regmap_divider_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_clk_tmp_sel.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_usb2_48m_clk_tmp = {
	.data = &(struct clk_regmap_gate_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.bit_idx = 26,
	},
	.hw.init = &(struct clk_init_data){
		.name = "usb2_48m_tmp",
		.ops = &clk_regmap_gate_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_clk_tmp_div.hw
		},
		.num_parents = 1,
		.flags = CLK_SET_RATE_PARENT
	},
};

static struct clk_regmap gxlx4_usb2_48m_clk = {
	.data = &(struct clk_regmap_mux_data){
		.offset = CLKCTRL_USB_CLK_CTRL0,
		.mask = 0x1,
		.shift = 27,
	},
	.hw.init = &(struct clk_init_data) {
		.name = "usb2_48m_clk",
		.ops = &clk_regmap_mux_ops,
		.parent_hws = (const struct clk_hw *[]) {
			&gxlx4_usb2_48m_clk_tmp.hw,
			&gxlx4_usb2_48m_pre.hw
		},
		.num_parents = 2,
		.flags = CLK_SET_RATE_PARENT
	},
};

#define MESON_GXLX4_SYS_GATE(_name, _reg, _bit)				\
struct clk_regmap _name = {						\
	.data = &(struct clk_regmap_gate_data) {			\
		.offset = (_reg),					\
		.bit_idx = (_bit),					\
	},								\
	.hw.init = &(struct clk_init_data) {				\
		.name = #_name,						\
		.ops = &clk_regmap_gate_ops,				\
		.parent_hws = (const struct clk_hw *[]) {		\
			&gxlx4_sys_clk.hw					\
		},							\
		.num_parents = 1,					\
		.flags = CLK_IGNORE_UNUSED,				\
	},								\
}

static MESON_GXLX4_SYS_GATE(gxlx4_sys_dos,			CLKCTRL_SYS_CLK_EN0_REG0, 0);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_demod,		CLKCTRL_SYS_CLK_EN0_REG0, 1);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_mali,			CLKCTRL_SYS_CLK_EN0_REG0, 2);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_iotm,			CLKCTRL_SYS_CLK_EN0_REG0, 3);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_am2axi1,		CLKCTRL_SYS_CLK_EN0_REG0, 7);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_msr_clk,		CLKCTRL_SYS_CLK_EN0_REG0, 15);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_audio,		CLKCTRL_SYS_CLK_EN0_REG0, 17);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_tvfe,			CLKCTRL_SYS_CLK_EN0_REG0, 18);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ethmac,		CLKCTRL_SYS_CLK_EN0_REG0, 19);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ethphy,		CLKCTRL_SYS_CLK_EN0_REG0, 20);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ethaxi,		CLKCTRL_SYS_CLK_EN0_REG0, 21);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ge2d,			CLKCTRL_SYS_CLK_EN0_REG0, 31);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_i2c_monitor,		CLKCTRL_SYS_CLK_EN0_REG1, 0);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_usb_u22h,		CLKCTRL_SYS_CLK_EN0_REG1, 1);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_usb_u2drd,		CLKCTRL_SYS_CLK_EN0_REG1, 2);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_usb_u2h,		CLKCTRL_SYS_CLK_EN0_REG1, 3);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_hdmirx20_aes,		CLKCTRL_SYS_CLK_EN0_REG1, 9);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_hdmirx_pclk,		CLKCTRL_SYS_CLK_EN0_REG1, 11);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_mmc_phy,		CLKCTRL_SYS_CLK_EN0_REG1, 12);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_mmc_dmc,		CLKCTRL_SYS_CLK_EN0_REG1, 13);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_pclk_cpu_apb,		CLKCTRL_SYS_CLK_EN0_REG1, 14);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_atv_demod,		CLKCTRL_SYS_CLK_EN0_REG1, 15);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_adec_top,		CLKCTRL_SYS_CLK_EN0_REG1, 16);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_vpu_intr,		CLKCTRL_SYS_CLK_EN0_REG1, 17);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_tcon,			CLKCTRL_SYS_CLK_EN0_REG1, 30);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_saradc,		CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 0);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_aocpu,		CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 1);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_gic,			CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 2);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_acodec,		CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 3);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_spisg,		CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 4);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_sd_emmc_c,		CLKCTRL_CLK81_GATE_CTRL_SYSWRAPPER, 5);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_uart_wrapper,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 0);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_pwm_wrapper,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 1);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_i2c_m_wrapper,	CLKCTRL_CLK81_GATE_CTRL_PERIPH, 2);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_i2c_s_a,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 3);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ir_crtl,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 4);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_smart_card,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 5);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ciplus,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 6);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_cec,			CLKCTRL_CLK81_GATE_CTRL_PERIPH, 7);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ts_cpu,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 8);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_ts_top,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 9);
static MESON_GXLX4_SYS_GATE(gxlx4_sys_led_ctrl,		CLKCTRL_CLK81_GATE_CTRL_PERIPH, 10);

/* Array of all clocks provided by this provider */
static struct clk_hw_onecell_data gxlx4_hw_onecell_data = {
	.hws = {
		[CLKID_FIXED_PLL]			= &gxlx4_fixed_pll.hw,
		[CLKID_FCLK_DIV2_DIV]			= &gxlx4_fclk_div2_div.hw,
		[CLKID_FCLK_DIV2]			= &gxlx4_fclk_div2.hw,
		[CLKID_FCLK_DIV3_DIV]			= &gxlx4_fclk_div3_div.hw,
		[CLKID_FCLK_DIV3]			= &gxlx4_fclk_div3.hw,
		[CLKID_FCLK_DIV4_DIV]			= &gxlx4_fclk_div4_div.hw,
		[CLKID_FCLK_DIV4]			= &gxlx4_fclk_div4.hw,
		[CLKID_FCLK_DIV5_DIV]			= &gxlx4_fclk_div5_div.hw,
		[CLKID_FCLK_DIV5]			= &gxlx4_fclk_div5.hw,
		[CLKID_FCLK_DIV7_DIV]			= &gxlx4_fclk_div7_div.hw,
		[CLKID_FCLK_DIV7]			= &gxlx4_fclk_div7.hw,
		[CLKID_FCLK_DIV2P5_DIV]			= &gxlx4_fclk_div2p5_div.hw,
		[CLKID_FCLK_DIV2P5]			= &gxlx4_fclk_div2p5.hw,
		[CLKID_GP0_PLL]				= &gxlx4_gp0_pll.hw,
		[CLKID_HIFI_PLL]			= &gxlx4_hifi_pll.hw,
		[CLKID_HIFI1_PLL]			= &gxlx4_hifi1_pll.hw,
		[CLKID_USB_PLL_DCO]			= &gxlx4_usb_pll_dco.hw,
		[CLKID_USB_PLL]				= &gxlx4_usb_pll.hw,
		[CLKID_MPLL_50M_DIV]			= &gxlx4_mpll_50m_div.hw,
		[CLKID_MPLL_50M]			= &gxlx4_mpll_50m.hw,
		[CLKID_RTC_32K_IN]			= &gxlx4_rtc_32k_in.hw,
		[CLKID_RTC_32K_DIV]			= &gxlx4_rtc_32k_div.hw,
		[CLKID_RTC_32K_FORCE_SEL]		= &gxlx4_rtc_32k_force_sel.hw,
		[CLKID_RTC_32K_OUT]			= &gxlx4_rtc_32k_out.hw,
		[CLKID_RTC_32K_MUX0_0]			= &gxlx4_rtc_32k_mux0_0.hw,
		[CLKID_RTC_32K_MUX0_1]			= &gxlx4_rtc_32k_mux0_1.hw,
		[CLKID_RTC]				= &gxlx4_rtc_clk.hw,
		[CLKID_SYS_CLK_0_SEL]			= &gxlx4_sys_clk_0_sel.hw,
		[CLKID_SYS_CLK_0_DIV]			= &gxlx4_sys_clk_0_div.hw,
		[CLKID_SYS_CLK_0]			= &gxlx4_sys_clk_0.hw,
		[CLKID_SYS_CLK_1_SEL]			= &gxlx4_sys_clk_1_sel.hw,
		[CLKID_SYS_CLK_1_DIV]			= &gxlx4_sys_clk_1_div.hw,
		[CLKID_SYS_CLK_1]			= &gxlx4_sys_clk_1.hw,
		[CLKID_SYS_CLK]				= &gxlx4_sys_clk.hw,
		[CLKID_AXI_CLK_0_SEL]			= &gxlx4_axi_clk_0_sel.hw,
		[CLKID_AXI_CLK_0_DIV]			= &gxlx4_axi_clk_0_div.hw,
		[CLKID_AXI_CLK_0]			= &gxlx4_axi_clk_0.hw,
		[CLKID_AXI_CLK_1_SEL]			= &gxlx4_axi_clk_1_sel.hw,
		[CLKID_AXI_CLK_1_DIV]			= &gxlx4_axi_clk_1_div.hw,
		[CLKID_AXI_CLK_1]			= &gxlx4_axi_clk_1.hw,
		[CLKID_AXI_CLK]				= &gxlx4_axi_clk.hw,
		[CLKID_SARADC_SEL]			= &gxlx4_saradc_sel.hw,
		[CLKID_SARADC_DIV]			= &gxlx4_saradc_div.hw,
		[CLKID_SARADC]				= &gxlx4_saradc.hw,
		[CLKID_CECB_32K_CLKIN]			= &gxlx4_cecb_32k_clkin.hw,
		[CLKID_CECB_32K_DIV]			= &gxlx4_cecb_32k_div.hw,
		[CLKID_CECB_32K_SEL_PRE]		= &gxlx4_cecb_32k_sel_pre.hw,
		[CLKID_CECB_32K_SEL]			= &gxlx4_cecb_32k_sel.hw,
		[CLKID_CECB_32K_CLKOUT]			= &gxlx4_cecb_32k_clkout.hw,
		[CLKID_SC_CLK_SEL]			= &gxlx4_sc_clk_sel.hw,
		[CLKID_SC_CLK_DIV]			= &gxlx4_sc_clk_div.hw,
		[CLKID_SC_CLK]				= &gxlx4_sc_clk.hw,
		[CLKID_24M_CLK_GATE]			= &gxlx4_24m_clk_gate.hw,
		[CLKID_24M_DIV2]			= &gxlx4_24m_div2.hw,
		[CLKID_12M_CLK]				= &gxlx4_12m_clk.hw,
		[CLKID_25M_CLK_DIV]			= &gxlx4_25m_clk_div.hw,
		[CLKID_25M_CLK]				= &gxlx4_25m_clk.hw,
		[CLKID_VCLK_SEL]			= &gxlx4_vclk_sel.hw,
		[CLKID_VCLK2_SEL]			= &gxlx4_vclk2_sel.hw,
		[CLKID_VCLK_INPUT]			= &gxlx4_vclk_input.hw,
		[CLKID_VCLK2_INPUT]			= &gxlx4_vclk2_input.hw,
		[CLKID_VCLK_DIV]			= &gxlx4_vclk_div.hw,
		[CLKID_VCLK2_DIV]			= &gxlx4_vclk2_div.hw,
		[CLKID_VCLK]				= &gxlx4_vclk.hw,
		[CLKID_VCLK2]				= &gxlx4_vclk2.hw,
		[CLKID_VCLK_DIV1]			= &gxlx4_vclk_div1.hw,
		[CLKID_VCLK_DIV2_EN]			= &gxlx4_vclk_div2_en.hw,
		[CLKID_VCLK_DIV4_EN]			= &gxlx4_vclk_div4_en.hw,
		[CLKID_VCLK_DIV6_EN]			= &gxlx4_vclk_div6_en.hw,
		[CLKID_VCLK_DIV12_EN]			= &gxlx4_vclk_div12_en.hw,
		[CLKID_VCLK2_DIV1]			= &gxlx4_vclk2_div1.hw,
		[CLKID_VCLK2_DIV2_EN]			= &gxlx4_vclk2_div2_en.hw,
		[CLKID_VCLK2_DIV4_EN]			= &gxlx4_vclk2_div4_en.hw,
		[CLKID_VCLK2_DIV6_EN]			= &gxlx4_vclk2_div6_en.hw,
		[CLKID_VCLK2_DIV12_EN]			= &gxlx4_vclk2_div12_en.hw,
		[CLKID_VCLK_DIV2]			= &gxlx4_vclk_div2.hw,
		[CLKID_VCLK_DIV4]			= &gxlx4_vclk_div4.hw,
		[CLKID_VCLK_DIV6]			= &gxlx4_vclk_div6.hw,
		[CLKID_VCLK_DIV12]			= &gxlx4_vclk_div12.hw,
		[CLKID_VCLK2_DIV2]			= &gxlx4_vclk2_div2.hw,
		[CLKID_VCLK2_DIV4]			= &gxlx4_vclk2_div4.hw,
		[CLKID_VCLK2_DIV6]			= &gxlx4_vclk2_div6.hw,
		[CLKID_VCLK2_DIV12]			= &gxlx4_vclk2_div12.hw,
		[CLKID_CTS_ENCI_SEL]			= &gxlx4_cts_enci_sel.hw,
		[CLKID_CTS_VDAC_SEL]			= &gxlx4_cts_vdac_sel.hw,
		[CLKID_HDMI_TX_SEL]			= &gxlx4_hdmi_tx_sel.hw,
		[CLKID_CTS_ENCI]			= &gxlx4_cts_enci.hw,
		[CLKID_CTS_ENCP]			= &gxlx4_cts_encp.hw,
		[CLKID_CTS_VDAC]			= &gxlx4_cts_vdac.hw,
		[CLKID_HDMI_TX]				= &gxlx4_hdmi_tx.hw,
		[CLKID_MALI_0_SEL]			= &gxlx4_mali_0_sel.hw,
		[CLKID_MALI_0_DIV]			= &gxlx4_mali_0_div.hw,
		[CLKID_MALI_0]				= &gxlx4_mali_0.hw,
		[CLKID_MALI_1_SEL]			= &gxlx4_mali_1_sel.hw,
		[CLKID_MALI_1_DIV]			= &gxlx4_mali_1_div.hw,
		[CLKID_MALI_1]				= &gxlx4_mali_1.hw,
		[CLKID_MALI]				= &gxlx4_mali.hw,
		[CLKID_HEVCB_0_SEL]			= &gxlx4_hevcb_0_sel.hw,
		[CLKID_HEVCB_0_DIV]			= &gxlx4_hevcb_0_div.hw,
		[CLKID_HEVCB_0]				= &gxlx4_hevcb_0.hw,
		[CLKID_HEVCB_1_SEL]			= &gxlx4_hevcb_1_sel.hw,
		[CLKID_HEVCB_1_DIV]			= &gxlx4_hevcb_1_div.hw,
		[CLKID_HEVCB_1]				= &gxlx4_hevcb_1.hw,
		[CLKID_HEVCB]				= &gxlx4_hevcb.hw,
		[CLKID_VPU_0_SEL]			= &gxlx4_vpu_0_sel.hw,
		[CLKID_VPU_0_DIV]			= &gxlx4_vpu_0_div.hw,
		[CLKID_VPU_0]				= &gxlx4_vpu_0.hw,
		[CLKID_VPU_1_SEL]			= &gxlx4_vpu_1_sel.hw,
		[CLKID_VPU_1_DIV]			= &gxlx4_vpu_1_div.hw,
		[CLKID_VPU_1]				= &gxlx4_vpu_1.hw,
		[CLKID_VPU]				= &gxlx4_vpu.hw,
		[CLKID_VPU_CLKB_TMP_SEL]		= &gxlx4_vpu_clkb_tmp_sel.hw,
		[CLKID_VPU_CLKB_TMP_DIV]		= &gxlx4_vpu_clkb_tmp_div.hw,
		[CLKID_VPU_CLKB_TMP]			= &gxlx4_vpu_clkb_tmp.hw,
		[CLKID_VPU_CLKB_DIV]			= &gxlx4_vpu_clkb_div.hw,
		[CLKID_VPU_CLKB]			= &gxlx4_vpu_clkb.hw,
		[CLKID_VAPB_0_SEL]			= &gxlx4_vapb_0_sel.hw,
		[CLKID_VAPB_0_DIV]			= &gxlx4_vapb_0_div.hw,
		[CLKID_VAPB_0]				= &gxlx4_vapb_0.hw,
		[CLKID_VAPB_1_SEL]			= &gxlx4_vapb_1_sel.hw,
		[CLKID_VAPB_1_DIV]			= &gxlx4_vapb_1_div.hw,
		[CLKID_VAPB_1]				= &gxlx4_vapb_1.hw,
		[CLKID_VAPB]				= &gxlx4_vapb.hw,
		[CLKID_GE2D_GATE]			= &gxlx4_ge2d_gate.hw,
		[CLKID_HDMIRX_5M_SEL]			= &gxlx4_hdmirx_5m_sel.hw,
		[CLKID_HDMIRX_5M_DIV]			= &gxlx4_hdmirx_5m_div.hw,
		[CLKID_HDMIRX_5M]			= &gxlx4_hdmirx_5m.hw,
		[CLKID_HDMIRX_2M_SEL]			= &gxlx4_hdmirx_2m_sel.hw,
		[CLKID_HDMIRX_2M_DIV]			= &gxlx4_hdmirx_2m_div.hw,
		[CLKID_HDMIRX_2M]			= &gxlx4_hdmirx_2m.hw,
		[CLKID_HDMIRX_CFG_SEL]			= &gxlx4_hdmirx_cfg_sel.hw,
		[CLKID_HDMIRX_CFG_DIV]			= &gxlx4_hdmirx_cfg_div.hw,
		[CLKID_HDMIRX_CFG]			= &gxlx4_hdmirx_cfg.hw,
		[CLKID_HDMIRX_HDCP_SEL]			= &gxlx4_hdmirx_hdcp_sel.hw,
		[CLKID_HDMIRX_HDCP_DIV]			= &gxlx4_hdmirx_hdcp_div.hw,
		[CLKID_HDMIRX_HDCP]			= &gxlx4_hdmirx_hdcp.hw,
		[CLKID_HDMIRX_AUD_PLL_SEL]		= &gxlx4_hdmirx_aud_pll_sel.hw,
		[CLKID_HDMIRX_AUD_PLL_DIV]		= &gxlx4_hdmirx_aud_pll_div.hw,
		[CLKID_HDMIRX_AUD_PLL]			= &gxlx4_hdmirx_aud_pll.hw,
		[CLKID_HDMIRX_ACR_SEL]			= &gxlx4_hdmirx_acr_sel.hw,
		[CLKID_HDMIRX_ACR_DIV]			= &gxlx4_hdmirx_acr_div.hw,
		[CLKID_HDMIRX_ACR]			= &gxlx4_hdmirx_acr.hw,
		[CLKID_HDMIRX_METER_SEL]		= &gxlx4_hdmirx_meter_sel.hw,
		[CLKID_HDMIRX_METER_DIV]		= &gxlx4_hdmirx_meter_div.hw,
		[CLKID_HDMIRX_METER]			= &gxlx4_hdmirx_meter.hw,
		[CLKID_PWM_A_SEL]			= &gxlx4_pwm_a_sel.hw,
		[CLKID_PWM_A_DIV]			= &gxlx4_pwm_a_div.hw,
		[CLKID_PWM_A]				= &gxlx4_pwm_a.hw,
		[CLKID_PWM_B_SEL]			= &gxlx4_pwm_b_sel.hw,
		[CLKID_PWM_B_DIV]			= &gxlx4_pwm_b_div.hw,
		[CLKID_PWM_B]				= &gxlx4_pwm_b.hw,
		[CLKID_PWM_C_SEL]			= &gxlx4_pwm_c_sel.hw,
		[CLKID_PWM_C_DIV]			= &gxlx4_pwm_c_div.hw,
		[CLKID_PWM_C]				= &gxlx4_pwm_c.hw,
		[CLKID_PWM_D_SEL]			= &gxlx4_pwm_d_sel.hw,
		[CLKID_PWM_D_DIV]			= &gxlx4_pwm_d_div.hw,
		[CLKID_PWM_D]				= &gxlx4_pwm_d.hw,
		[CLKID_PWM_E_SEL]			= &gxlx4_pwm_e_sel.hw,
		[CLKID_PWM_E_DIV]			= &gxlx4_pwm_e_div.hw,
		[CLKID_PWM_E]				= &gxlx4_pwm_e.hw,
		[CLKID_PWM_F_SEL]			= &gxlx4_pwm_f_sel.hw,
		[CLKID_PWM_F_DIV]			= &gxlx4_pwm_f_div.hw,
		[CLKID_PWM_F]				= &gxlx4_pwm_f.hw,
		[CLKID_PWM_G_SEL]			= &gxlx4_pwm_g_sel.hw,
		[CLKID_PWM_G_DIV]			= &gxlx4_pwm_g_div.hw,
		[CLKID_PWM_G]				= &gxlx4_pwm_g.hw,
		[CLKID_PWM_H_SEL]			= &gxlx4_pwm_h_sel.hw,
		[CLKID_PWM_H_DIV]			= &gxlx4_pwm_h_div.hw,
		[CLKID_PWM_H]				= &gxlx4_pwm_h.hw,
		[CLKID_SPICC0_SEL]			= &gxlx4_spicc0_sel.hw,
		[CLKID_SPICC0_DIV]			= &gxlx4_spicc0_div.hw,
		[CLKID_SPICC0]				= &gxlx4_spicc0.hw,
		[CLKID_SPICC1_SEL]			= &gxlx4_spicc1_sel.hw,
		[CLKID_SPICC1_DIV]			= &gxlx4_spicc1_div.hw,
		[CLKID_SPICC1]				= &gxlx4_spicc1.hw,
		[CLKID_SD_EMMC_C_SEL]			= &gxlx4_sd_emmc_c_sel.hw,
		[CLKID_SD_EMMC_C_DIV]			= &gxlx4_sd_emmc_c_div.hw,
		[CLKID_SD_EMMC_C]			= &gxlx4_sd_emmc_c.hw,
		[CLKID_VDIN_MEAS_SEL]			= &gxlx4_vdin_meas_sel.hw,
		[CLKID_VDIN_MEAS_DIV]			= &gxlx4_vdin_meas_div.hw,
		[CLKID_VDIN_MEAS]			= &gxlx4_vdin_meas.hw,
		[CLKID_VID_LOCK_DIV]			= &gxlx4_vid_lock_div.hw,
		[CLKID_VID_LOCK]			= &gxlx4_vid_lock_clk.hw,
		[CLKID_ETH_RMII_SEL]			= &gxlx4_eth_rmii_sel.hw,
		[CLKID_ETH_RMII_DIV]			= &gxlx4_eth_rmii_div.hw,
		[CLKID_ETH_RMII]			= &gxlx4_eth_rmii.hw,
		[CLKID_ETH_DIV8]			= &gxlx4_eth_div8.hw,
		[CLKID_ETH_125M]			= &gxlx4_eth_125m.hw,
		[CLKID_TS_CLK_DIV]			= &gxlx4_ts_clk_div.hw,
		[CLKID_TS_CLK]				= &gxlx4_ts_clk.hw,
		[CLKID_CTS_TCON_PLL_CLK_SEL]		= &gxlx4_cts_tcon_pll_clk_sel.hw,
		[CLKID_CTS_TCON_PLL_CLK_DIV]		= &gxlx4_cts_tcon_pll_clk_div.hw,
		[CLKID_CTS_TCON_PLL_CLK]		= &gxlx4_cts_tcon_pll_clk.hw,
		[CLKID_ADC_EXTCLK_SEL]			= &gxlx4_adc_extclk_sel.hw,
		[CLKID_ADC_EXTCLK_DIV]			= &gxlx4_adc_extclk_div.hw,
		[CLKID_ADC_EXTCLK]			= &gxlx4_adc_extclk.hw,
		[CLKID_DEMOD_32K_CLKIN]			= &gxlx4_demod_32k_clkin.hw,
		[CLKID_DEMOD_32K_DIV]			= &gxlx4_demod_32k_div.hw,
		[CLKID_DEMOD_32K]			= &gxlx4_demod_32k.hw,
		[CLKID_CTS_DEMOD_CORE_SEL]		= &gxlx4_cts_demod_core_sel.hw,
		[CLKID_CTS_DEMOD_CORE_DIV]		= &gxlx4_cts_demod_core_div.hw,
		[CLKID_CTS_DEMOD_CORE]			= &gxlx4_cts_demod_core.hw,
		[CLKID_CTS_DEMOD_CORE_T2_SEL]		= &gxlx4_cts_demod_core_t2_clk_sel.hw,
		[CLKID_CTS_DEMOD_CORE_T2_DIV]		= &gxlx4_cts_demod_core_t2_clk_div.hw,
		[CLKID_CTS_DEMOD_CORE_T2]		= &gxlx4_cts_demod_core_t2_clk.hw,
		[CLKID_CDAC_CLK_SEL]			= &gxlx4_cdac_sel.hw,
		[CLKID_CDAC_CLK_DIV]			= &gxlx4_cdac_div.hw,
		[CLKID_CDAC_CLK]			= &gxlx4_cdac.hw,
		[CLKID_TSIN_0_SEL]			= &gxlx4_tsin_0_sel.hw,
		[CLKID_TSIN_0_DIV]			= &gxlx4_tsin_0_div.hw,
		[CLKID_TSIN_0]				= &gxlx4_tsin_0.hw,
		[CLKID_TSIN_1_SEL]			= &gxlx4_tsin_1_sel.hw,
		[CLKID_TSIN_1_DIV]			= &gxlx4_tsin_1_div.hw,
		[CLKID_TSIN_1]				= &gxlx4_tsin_1.hw,
		[CLKID_TSIN_SEL]			= &gxlx4_tsin_sel.hw,
		[CLKID_TSIN]				= &gxlx4_tsin.hw,
		[CLKID_AMFC_0_SEL]			= &gxlx4_amfc_0_sel.hw,
		[CLKID_AMFC_0_DIV]			= &gxlx4_amfc_0_div.hw,
		[CLKID_AMFC_0]				= &gxlx4_amfc_0.hw,
		[CLKID_AMFC_1_SEL]			= &gxlx4_amfc_1_sel.hw,
		[CLKID_AMFC_1_DIV]			= &gxlx4_amfc_1_div.hw,
		[CLKID_AMFC_1]				= &gxlx4_amfc_1.hw,
		[CLKID_AMFC]				= &gxlx4_amfc.hw,
		[CLKID_USB2_48M_PRE_IN]			= &gxlx4_usb2_48m_pre_in.hw,
		[CLKID_USB2_48M_PRE_DIV]		= &gxlx4_usb2_48m_pre_div.hw,
		[CLKID_USB2_48M_PRE_FORCE_SEL]		= &gxlx4_usb2_48m_pre_force_sel.hw,
		[CLKID_USB2_48M_PRE]			= &gxlx4_usb2_48m_pre.hw,
		[CLKID_USB2_48M_CLK_TMP_SEL]		= &gxlx4_usb2_48m_clk_tmp_sel.hw,
		[CLKID_USB2_48M_CLK_TMP_DIV]		= &gxlx4_usb2_48m_clk_tmp_div.hw,
		[CLKID_USB2_48M_CLK_TMP]		= &gxlx4_usb2_48m_clk_tmp.hw,
		[CLKID_USB2_48M_CLK]			= &gxlx4_usb2_48m_clk.hw,
		[CLKID_GEN_SEL]				= &gxlx4_gen_sel.hw,
		[CLKID_GEN_DIV]				= &gxlx4_gen_div.hw,
		[CLKID_GEN]				= &gxlx4_gen.hw,
		[CLKID_SYS_DOS]				= &gxlx4_sys_dos.hw,
		[CLKID_SYS_DEMOD]			= &gxlx4_sys_demod.hw,
		[CLKID_SYS_MALI]			= &gxlx4_sys_mali.hw,
		[CLKID_SYS_IOTM]			= &gxlx4_sys_iotm.hw,
		[CLKID_SYS_AM2AXI]			= &gxlx4_sys_am2axi1.hw,
		[CLKID_SYS_MSR_CLK]			= &gxlx4_sys_msr_clk.hw,
		[CLKID_SYS_AUDIO]			= &gxlx4_sys_audio.hw,
		[CLKID_SYS_TVFE]			= &gxlx4_sys_tvfe.hw,
		[CLKID_SYS_ETHMAC]			= &gxlx4_sys_ethmac.hw,
		[CLKID_SYS_ETHPHY]			= &gxlx4_sys_ethphy.hw,
		[CLKID_SYS_ETHAXI]			= &gxlx4_sys_ethaxi.hw,
		[CLKID_SYS_GE2D]			= &gxlx4_sys_ge2d.hw,
		[CLKID_SYS_I2C_MONITOR]			= &gxlx4_sys_i2c_monitor.hw,
		[CLKID_SYS_USB_U22H]			= &gxlx4_sys_usb_u22h.hw,
		[CLKID_SYS_USB_U2DRD]			= &gxlx4_sys_usb_u2drd.hw,
		[CLKID_SYS_USB_U2H]			= &gxlx4_sys_usb_u2h.hw,
		[CLKID_SYS_HDMIRX20_AES]		= &gxlx4_sys_hdmirx20_aes.hw,
		[CLKID_SYS_HDMIRX_PCLK]			= &gxlx4_sys_hdmirx_pclk.hw,
		[CLKID_SYS_MMC_PHY]			= &gxlx4_sys_mmc_phy.hw,
		[CLKID_SYS_MMC_DMC]			= &gxlx4_sys_mmc_dmc.hw,
		[CLKID_SYS_PCLK_SYS_CPU_APB]		= &gxlx4_sys_pclk_cpu_apb.hw,
		[CLKID_SYS_ATV_DEMOD]			= &gxlx4_sys_atv_demod.hw,
		[CLKID_SYS_ADEC_TOP]			= &gxlx4_sys_adec_top.hw,
		[CLKID_SYS_VPU_INTR]			= &gxlx4_sys_vpu_intr.hw,
		[CLKID_SYS_TCON]			= &gxlx4_sys_tcon.hw,
		[CLKID_SYS_SARADC]			= &gxlx4_sys_saradc.hw,
		[CLKID_SYS_AOCPU]			= &gxlx4_sys_aocpu.hw,
		[CLKID_SYS_GIC]				= &gxlx4_sys_gic.hw,
		[CLKID_SYS_ACODEC]			= &gxlx4_sys_acodec.hw,
		[CLKID_SYS_SPISG]			= &gxlx4_sys_spisg.hw,
		[CLKID_SYS_SD_EMMC_C]			= &gxlx4_sys_sd_emmc_c.hw,
		[CLKID_SYS_UART_WRAPPER]		= &gxlx4_sys_uart_wrapper.hw,
		[CLKID_SYS_PWM_WRAPPER]			= &gxlx4_sys_pwm_wrapper.hw,
		[CLKID_SYS_I2C_M_WRAPPER]		= &gxlx4_sys_i2c_m_wrapper.hw,
		[CLKID_SYS_I2C_S_A]			= &gxlx4_sys_i2c_s_a.hw,
		[CLKID_SYS_IR__CTRL]			= &gxlx4_sys_ir_crtl.hw,
		[CLKID_SYS_SMART_CARD]			= &gxlx4_sys_smart_card.hw,
		[CLKID_SYS_CIPLUS]			= &gxlx4_sys_ciplus.hw,
		[CLKID_SYS_CEC]				= &gxlx4_sys_cec.hw,
		[CLKID_SYS_TS_CPU]			= &gxlx4_sys_ts_cpu.hw,
		[CLKID_SYS_TS_TOP]			= &gxlx4_sys_ts_top.hw,
		[CLKID_SYS_LED_CTRL]			= &gxlx4_sys_led_ctrl.hw,
		[NR_CLKS]				= NULL
	},
	.num = NR_CLKS,
};

/* Convenience table to populate regmap in .probe */
static struct clk_regmap *const gxlx4_clk_regmaps[] = {
	&gxlx4_rtc_32k_in,
	&gxlx4_rtc_32k_div,
	&gxlx4_rtc_32k_force_sel,
	&gxlx4_rtc_32k_out,
	&gxlx4_rtc_32k_mux0_0,
	&gxlx4_rtc_32k_mux0_1,
	&gxlx4_rtc_clk,
	&gxlx4_sys_clk_0_sel,
	&gxlx4_sys_clk_0_div,
	&gxlx4_sys_clk_0,
	&gxlx4_sys_clk_1_sel,
	&gxlx4_sys_clk_1_div,
	&gxlx4_sys_clk_1,
	&gxlx4_sys_clk,
	&gxlx4_axi_clk_0_sel,
	&gxlx4_axi_clk_0_div,
	&gxlx4_axi_clk_0,
	&gxlx4_axi_clk_1_sel,
	&gxlx4_axi_clk_1_div,
	&gxlx4_axi_clk_1,
	&gxlx4_axi_clk,
	&gxlx4_saradc_sel,
	&gxlx4_saradc_div,
	&gxlx4_saradc,
	&gxlx4_cecb_32k_clkin,
	&gxlx4_cecb_32k_div,
	&gxlx4_cecb_32k_sel_pre,
	&gxlx4_cecb_32k_sel,
	&gxlx4_cecb_32k_clkout,
	&gxlx4_sc_clk_sel,
	&gxlx4_sc_clk_div,
	&gxlx4_sc_clk,
	&gxlx4_vclk_sel,
	&gxlx4_vclk2_sel,
	&gxlx4_vclk_input,
	&gxlx4_vclk2_input,
	&gxlx4_vclk_div,
	&gxlx4_vclk2_div,
	&gxlx4_vclk,
	&gxlx4_vclk2,
	&gxlx4_vclk_div1,
	&gxlx4_vclk_div2_en,
	&gxlx4_vclk_div4_en,
	&gxlx4_vclk_div6_en,
	&gxlx4_vclk_div12_en,
	&gxlx4_vclk2_div1,
	&gxlx4_vclk2_div2_en,
	&gxlx4_vclk2_div4_en,
	&gxlx4_vclk2_div6_en,
	&gxlx4_vclk2_div12_en,
	&gxlx4_cts_enci_sel,
	&gxlx4_cts_vdac_sel,
	&gxlx4_hdmi_tx_sel,
	&gxlx4_cts_enci,
	&gxlx4_cts_encp,
	&gxlx4_cts_vdac,
	&gxlx4_hdmi_tx,
	&gxlx4_mali_0_sel,
	&gxlx4_mali_0_div,
	&gxlx4_mali_0,
	&gxlx4_mali_1_sel,
	&gxlx4_mali_1_div,
	&gxlx4_mali_1,
	&gxlx4_mali,
	&gxlx4_hevcb_0_sel,
	&gxlx4_hevcb_0_div,
	&gxlx4_hevcb_0,
	&gxlx4_hevcb_1_sel,
	&gxlx4_hevcb_1_div,
	&gxlx4_hevcb_1,
	&gxlx4_hevcb,
	&gxlx4_vpu_0_sel,
	&gxlx4_vpu_0_div,
	&gxlx4_vpu_0,
	&gxlx4_vpu_1_sel,
	&gxlx4_vpu_1_div,
	&gxlx4_vpu_1,
	&gxlx4_vpu,
	&gxlx4_vpu_clkb_tmp_sel,
	&gxlx4_vpu_clkb_tmp_div,
	&gxlx4_vpu_clkb_tmp,
	&gxlx4_vpu_clkb_div,
	&gxlx4_vpu_clkb,
	&gxlx4_vapb_0_sel,
	&gxlx4_vapb_0_div,
	&gxlx4_vapb_0,
	&gxlx4_vapb_1_sel,
	&gxlx4_vapb_1_div,
	&gxlx4_vapb_1,
	&gxlx4_vapb,
	&gxlx4_ge2d_gate,
	&gxlx4_hdmirx_5m_sel,
	&gxlx4_hdmirx_5m_div,
	&gxlx4_hdmirx_5m,
	&gxlx4_hdmirx_2m_sel,
	&gxlx4_hdmirx_2m_div,
	&gxlx4_hdmirx_2m,
	&gxlx4_hdmirx_cfg_sel,
	&gxlx4_hdmirx_cfg_div,
	&gxlx4_hdmirx_cfg,
	&gxlx4_hdmirx_hdcp_sel,
	&gxlx4_hdmirx_hdcp_div,
	&gxlx4_hdmirx_hdcp,
	&gxlx4_hdmirx_aud_pll_sel,
	&gxlx4_hdmirx_aud_pll_div,
	&gxlx4_hdmirx_aud_pll,
	&gxlx4_hdmirx_acr_sel,
	&gxlx4_hdmirx_acr_div,
	&gxlx4_hdmirx_acr,
	&gxlx4_hdmirx_meter_sel,
	&gxlx4_hdmirx_meter_div,
	&gxlx4_hdmirx_meter,
	&gxlx4_pwm_a_sel,
	&gxlx4_pwm_a_div,
	&gxlx4_pwm_a,
	&gxlx4_pwm_b_sel,
	&gxlx4_pwm_b_div,
	&gxlx4_pwm_b,
	&gxlx4_pwm_c_sel,
	&gxlx4_pwm_c_div,
	&gxlx4_pwm_c,
	&gxlx4_pwm_d_sel,
	&gxlx4_pwm_d_div,
	&gxlx4_pwm_d,
	&gxlx4_pwm_e_sel,
	&gxlx4_pwm_e_div,
	&gxlx4_pwm_e,
	&gxlx4_pwm_f_sel,
	&gxlx4_pwm_f_div,
	&gxlx4_pwm_f,
	&gxlx4_pwm_g_sel,
	&gxlx4_pwm_g_div,
	&gxlx4_pwm_g,
	&gxlx4_pwm_h_sel,
	&gxlx4_pwm_h_div,
	&gxlx4_pwm_h,
	&gxlx4_spicc0_sel,
	&gxlx4_spicc0_div,
	&gxlx4_spicc0,
	&gxlx4_spicc1_sel,
	&gxlx4_spicc1_div,
	&gxlx4_spicc1,
	&gxlx4_sd_emmc_c_sel,
	&gxlx4_sd_emmc_c_div,
	&gxlx4_sd_emmc_c,
	&gxlx4_vdin_meas_sel,
	&gxlx4_vdin_meas_div,
	&gxlx4_vdin_meas,
	&gxlx4_vid_lock_div,
	&gxlx4_vid_lock_clk,
	&gxlx4_eth_rmii_sel,
	&gxlx4_eth_rmii_div,
	&gxlx4_eth_rmii,
	&gxlx4_eth_125m,
	&gxlx4_ts_clk_div,
	&gxlx4_ts_clk,
	&gxlx4_amfc_0_sel,
	&gxlx4_amfc_0_div,
	&gxlx4_amfc_0,
	&gxlx4_amfc_1_sel,
	&gxlx4_amfc_1_div,
	&gxlx4_amfc_1,
	&gxlx4_amfc,
	&gxlx4_usb2_48m_pre_in,
	&gxlx4_usb2_48m_pre_div,
	&gxlx4_usb2_48m_pre_force_sel,
	&gxlx4_usb2_48m_pre,
	&gxlx4_usb2_48m_clk_tmp_sel,
	&gxlx4_usb2_48m_clk_tmp_div,
	&gxlx4_usb2_48m_clk_tmp,
	&gxlx4_usb2_48m_clk,
	&gxlx4_cts_tcon_pll_clk_sel,
	&gxlx4_cts_tcon_pll_clk_div,
	&gxlx4_cts_tcon_pll_clk,
	&gxlx4_adc_extclk_sel,
	&gxlx4_adc_extclk_div,
	&gxlx4_adc_extclk,
	&gxlx4_demod_32k_clkin,
	&gxlx4_demod_32k_div,
	&gxlx4_demod_32k,
	&gxlx4_cts_demod_core_sel,
	&gxlx4_cts_demod_core_div,
	&gxlx4_cts_demod_core,
	&gxlx4_cts_demod_core_t2_clk_sel,
	&gxlx4_cts_demod_core_t2_clk_div,
	&gxlx4_cts_demod_core_t2_clk,
	&gxlx4_cdac_sel,
	&gxlx4_cdac_div,
	&gxlx4_cdac,
	&gxlx4_tsin_0_sel,
	&gxlx4_tsin_0_div,
	&gxlx4_tsin_0,
	&gxlx4_tsin_1_sel,
	&gxlx4_tsin_1_div,
	&gxlx4_tsin_1,
	&gxlx4_tsin_sel,
	&gxlx4_tsin,
	&gxlx4_gen_sel,
	&gxlx4_gen_div,
	&gxlx4_gen,
	&gxlx4_24m_clk_gate,
	&gxlx4_12m_clk,
	&gxlx4_25m_clk_div,
	&gxlx4_25m_clk,
	&gxlx4_sys_dos,
	&gxlx4_sys_demod,
	&gxlx4_sys_mali,
	&gxlx4_sys_iotm,
	&gxlx4_sys_am2axi1,
	&gxlx4_sys_msr_clk,
	&gxlx4_sys_audio,
	&gxlx4_sys_tvfe,
	&gxlx4_sys_ethmac,
	&gxlx4_sys_ethphy,
	&gxlx4_sys_ethaxi,
	&gxlx4_sys_ge2d,
	&gxlx4_sys_i2c_monitor,
	&gxlx4_sys_usb_u22h,
	&gxlx4_sys_usb_u2drd,
	&gxlx4_sys_usb_u2h,
	&gxlx4_sys_hdmirx20_aes,
	&gxlx4_sys_hdmirx_pclk,
	&gxlx4_sys_mmc_phy,
	&gxlx4_sys_mmc_dmc,
	&gxlx4_sys_pclk_cpu_apb,
	&gxlx4_sys_atv_demod,
	&gxlx4_sys_adec_top,
	&gxlx4_sys_vpu_intr,
	&gxlx4_sys_tcon,
	&gxlx4_sys_saradc,
	&gxlx4_sys_aocpu,
	&gxlx4_sys_gic,
	&gxlx4_sys_acodec,
	&gxlx4_sys_spisg,
	&gxlx4_sys_sd_emmc_c,
	&gxlx4_sys_uart_wrapper,
	&gxlx4_sys_pwm_wrapper,
	&gxlx4_sys_i2c_m_wrapper,
	&gxlx4_sys_i2c_s_a,
	&gxlx4_sys_ir_crtl,
	&gxlx4_sys_smart_card,
	&gxlx4_sys_ciplus,
	&gxlx4_sys_cec,
	&gxlx4_sys_ts_cpu,
	&gxlx4_sys_ts_top,
	&gxlx4_sys_led_ctrl,
};

static struct clk_regmap *const gxlx4_pll_clk_regmaps[] = {
	&gxlx4_fixed_pll,
	&gxlx4_fclk_div2,
	&gxlx4_fclk_div3,
	&gxlx4_fclk_div4,
	&gxlx4_fclk_div5,
	&gxlx4_fclk_div7,
	&gxlx4_fclk_div2p5,
	&gxlx4_gp0_pll,
	&gxlx4_hifi_pll,
	&gxlx4_hifi1_pll,
	&gxlx4_usb_pll_dco,
	&gxlx4_mpll_50m,
};


static const struct regmap_config gxlx4_clk_regmap_config = {
	.reg_bits	= 32,
	.val_bits	= 32,
	.reg_stride	= 4,
	.max_register	= 0x100000,
	.fast_io	= true,
};

/* vanilla replacement for the BSP meson_clk_regmap_resource() helper:
 * regmap for the idx-th memory resource of the device */
static struct regmap *gxlx4_regmap_resource(struct platform_device *pdev, int idx)
{
	struct resource *res;
	void __iomem *base;

	res = platform_get_resource(pdev, IORESOURCE_MEM, idx);
	if (!res)
		return ERR_PTR(-ENODEV);

	base = devm_platform_ioremap_resource(pdev, idx);
	if (IS_ERR(base))
		return base;

	return devm_regmap_init_mmio(&pdev->dev, base,
				     &gxlx4_clk_regmap_config);
}

static int meson_gxlx4_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct regmap *basic_map, *pll_map;
	int ret, i, ssc_size = 0;

	/* Get regmap for different clock area */
	basic_map = gxlx4_regmap_resource(pdev, 0);
	if (IS_ERR(basic_map)) {
		dev_err(dev, "basic clk registers not found\n");
		return PTR_ERR(basic_map);
	}

	pll_map = gxlx4_regmap_resource(pdev, 1);
	if (IS_ERR(pll_map)) {
		dev_err(dev, "pll clk registers not found\n");
		return PTR_ERR(pll_map);
	}

	/* Populate regmap for the regmap backed clocks */
	for (i = 0; i < ARRAY_SIZE(gxlx4_clk_regmaps); i++)
		gxlx4_clk_regmaps[i]->map = basic_map;

	for (i = 0; i < ARRAY_SIZE(gxlx4_pll_clk_regmaps); i++)
		gxlx4_pll_clk_regmaps[i]->map = pll_map;

	for (i = 0; i < gxlx4_hw_onecell_data.num; i++) {
		/* array might be sparse */
		if (!gxlx4_hw_onecell_data.hws[i])
			continue;

		pr_debug("registering %dth  %s\n", i,
			gxlx4_hw_onecell_data.hws[i]->init->name);

		ret = devm_clk_hw_register(dev, gxlx4_hw_onecell_data.hws[i]);
		if (ret) {
			dev_warn(dev, "clk %d (%s) register failed %d, skip\n",
				 i, gxlx4_hw_onecell_data.hws[i]->init->name, ret);
			gxlx4_hw_onecell_data.hws[i] = NULL;
			continue;
		}
#ifdef CONFIG_AMLOGIC_CLK_DEBUG
		ret = devm_clk_hw_register_clkdev(dev, gxlx4_hw_onecell_data.hws[i],
					  NULL,
					  clk_hw_get_name(gxlx4_hw_onecell_data.hws[i]));
		if (ret < 0) {
			dev_err(dev, "Failed to clkdev register: %d\n", ret);
			return ret;
		}
#endif
	}

	/* set gp0 emmc spread spectrum, Bits [26:23] of ANACTRL_GP0_PLL_CNTL2 control the size. */
	ret = of_property_read_u32(dev->of_node, "gp0-spread-spectrum", &ssc_size);
	if (ret)
		ssc_size = 0;

	regmap_update_bits(pll_map, ANACTRL_GP0PLL_CTRL2, 0xf << 23, (ssc_size / 500) << 23);

	return devm_of_clk_add_hw_provider(dev, of_clk_hw_onecell_get,
					   &gxlx4_hw_onecell_data);
}

static const struct of_device_id clkc_match_table[] = {
	{
		.compatible = "amlogic,gxlx4-clkc"
	},
	{}
};

static struct platform_driver gxlx4_driver = {
	.probe		= meson_gxlx4_probe,
	.driver		= {
		.name	= "gxlx4-clkc",
		.of_match_table = clkc_match_table,
	},
};

#if IS_BUILTIN(CONFIG_COMMON_CLK_GXLX4)
static int gxlx4_clkc_init(void)
{
	return platform_driver_register(&gxlx4_driver);
}
arch_initcall_sync(gxlx4_clkc_init);
#else
builtin_platform_driver(gxlx4_driver);
#endif

MODULE_LICENSE("GPL v2");

// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/usb/phy_companion.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/usb/phy.h>
#include <linux/amlogic/usb-v2.h>
#include "../usb_main.h"
#include <linux/amlogic/usbtype.h>
#include "../phy/phy-aml-new-usb-v2.h"

static struct amlogic_usb_v2	*g_c2_phy2;
char name_c[32];

static int amlogic_new_c2_usbphy_reset_v2(struct amlogic_usb_v2 *phy)
{
	static int	init_count;

	if (!init_count) {
		init_count++;
		writel((0x1 << phy->usb_reset_bit),
		       phy->reset_regs);
	}

	return 0;
}

static int amlogic_new_c2_usbphy_reset_phycfg_v2(struct amlogic_usb_v2 *phy, int cnt)
{
	u32 val, i = 0;
	u32 temp = 0;
	size_t mask = 0;

	mask = (size_t)phy->reset_regs & 0xf;

	for (i = 0; i < cnt; i++)
		temp = temp | (1 << phy->phy_reset_level_bit[i]);

	/* set usb phy to low power mode */
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));

	udelay(200);

	val = readl((void __iomem *)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));
	writel((val | temp), (void __iomem *)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));

	return 0;
}

static void usb_set_calibration_trim
	(void __iomem *reg, struct amlogic_usb_v2 *phy)
{
	u32 value = 0;
	u32 cali, i, tmp;
	u8 cali_en;

	if (!phy->usb_phy_trim_reg) {
		dev_err(phy->dev, "Not usb-phy-trim-reg\n");
		return;
	}

	cali = readl(phy->usb_phy_trim_reg);
	cali_en = (cali >> 30) & 0x1;
	tmp = cali >> 26;

	if (cali_en) {
		tmp = tmp & 0xf;
		cali = tmp;
		if (cali > 12)
			cali = 12;

		value = readl(reg + 0x10);
		value &= (~0xfff);
		for (i = 0; i < cali; i++)
			value |= (1 << i);

		writel(value, reg + 0x10);
	}

	dev_info(phy->dev, "phy trim value= %u\n", value);
}

static u32 set_usb_C2_pll(struct amlogic_usb_v2 *phy, void __iomem	*reg)
{
	struct u2p_aml_regs_v2 u2p_aml_regs;
	union usb_top_version_reg reg2;
	u32 val = 0;
	u32 retry = 5;

	u2p_aml_regs.u2p_r_v2[2] = phy->regs + 0x08;
	reg2.d32 = readl(u2p_aml_regs.u2p_r_v2[2]);
	dev_info(phy->dev, "phy version= %u, usb20 num =%u\n",
		 reg2.b.phy_version, reg2.b.USB20_port_num);

__retry:
	writel((USB_PHY2_RESET | (phy->pll_setting[0])), reg + 0x40);
	writel(phy->pll_setting[1], reg + 0x44);
	usleep_range(5, 10);
	writel((USB_PHY2_ENABLE | USB_PHY2_RESET | (phy->pll_setting[0])),
	       reg + 0x40);
	usleep_range(5, 10);
	writel((USB_PHY2_ENABLE | (phy->pll_setting[0])), reg + 0x40);
	usleep_range(10, 20);
	writel(USBPLL_LK_OD_EN | phy->pll_setting[1], reg + 0x44);
	usleep_range(200, 210);

	val = readl(reg + 0x40);
	if (val >> USBPLL_LOCKFLAG_BIT)
		return 0;

	retry--;
	if (!retry)
		return 1;
	goto __retry;
}

static void set_usb_pll(struct amlogic_usb_v2 *phy, void __iomem	*reg)
{
	u32 val;
	struct u2p_aml_regs_v2 u2p_aml_regs;
	union usb_top_version_reg reg2;

	if (!phy)
		return;

	u2p_aml_regs.u2p_r_v2[2] = phy->regs + 0x08;
	reg2.d32 = readl(u2p_aml_regs.u2p_r_v2[2]);

	if (phy->phy_version == 3 || reg2.b.phy_version) {
		set_usb_C2_pll(phy, reg);
	} else {
	/* TO DO set usb  PLL */
		writel((0x30000000 | (phy->pll_setting[0])), reg + 0x40);
		writel(phy->pll_setting[1], reg + 0x44);
		writel(phy->pll_setting[2], reg + 0x48);
		usleep_range(100, 120);
		writel((0x10000000 | (phy->pll_setting[0])), reg + 0x40);
	}

	/* PHY Tune */
	writel(TUNING_DISCONNECT_THRESHOLD, reg + 0xC);
	if (phy->phy_version) {
		writel(phy->pll_setting[3], reg + 0x50);
		writel(0x2a, reg + 0x54);

		val = readl(reg + 0x08);
		val &= 0xfff;
		writel(val | readl(reg + 0x10), reg + 0x10);

		writel(0x78000, reg + 0x34);
	} else {
		writel(phy->pll_setting[3], reg + 0x50);
		writel(phy->pll_setting[4], reg + 0x10);
		writel(0, reg + 0x38);
		writel(phy->pll_setting[5], reg + 0x34);
	}
}

void set_usb_phy_reg10(int mode)
{
	void __iomem	*reg;
	struct u2p_aml_regs_v2 u2p_aml_regs;
	union usb_top_version_reg reg2;
	u32 val;
	u32 port;

	if (!g_c2_phy2)
		return;
	/*** read Version_reg and the USB PHY version ==1, it is c2, need handle 0x10***/
	u2p_aml_regs.u2p_r_v2[2] = g_c2_phy2->regs + 0x08;
	reg2.d32 = readl(u2p_aml_regs.u2p_r_v2[2]);
	if (reg2.b.phy_version != USB_PHY_VERSION_C2)
		return;

	port = g_c2_phy2->otg_phy_index;
	reg = g_c2_phy2->phy_cfg[port];

	if (mode == HOST_MODE) {
		usb_set_calibration_trim(reg, g_c2_phy2);
		writel(g_c2_phy2->pll_setting[3], reg + 0x50);
	} else {
		val = 0xfff;
		writel(val | readl(reg + 0x10), reg + 0x10);
		writel(0xbe18, reg + 0x50);
	}
}
EXPORT_SYMBOL(set_usb_phy_reg10);

static int amlogic_new_c2_usb2_init(struct usb_phy *x)
{
	int i, j, cnt;

	struct amlogic_usb_v2 *phy;
	struct u2p_aml_regs_v2 u2p_aml_regs;
	union u2p_r0_v2 reg0;
	union u2p_r1_v2 reg1;
	u32 val;
	u32 temp = 0;
	int portnum;
	size_t mask = 0;

	phy = phy_to_amlusb(x);

	portnum = phy->portnum;

	mask = (size_t)phy->reset_regs & 0xf;

	if (phy->suspend_flag) {
		if (phy->phy.flags == AML_USB2_PHY_ENABLE)
			clk_prepare_enable(phy->clk);
	}

	for (i = 0; i < portnum; i++)
		temp = temp | (1 << phy->phy_reset_level_bit[i]);

	if (phy->phy_version == 3) {
		if (phy->suspend_flag == 0) {
			writel(temp, (void __iomem *)
			((unsigned long)phy->reset_regs));
		}
	}
	val = readl((void __iomem *)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));
	writel((val | temp), (void __iomem *)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));

	amlogic_new_c2_usbphy_reset_v2(phy);
	udelay(10);

	for (i = 0; i < phy->portnum; i++) {
		for (j = 0; j < 4; j++) {
			u2p_aml_regs.u2p_r_v2[j] = (void __iomem	*)
				((unsigned long)phy->regs + i * PHY_REGISTER_SIZE
				+ 4 * j);
		}

		reg0.d32 = readl(u2p_aml_regs.u2p_r_v2[0]);
		reg0.b.POR = 1;
		if (phy->suspend_flag == 0) {
			reg0.b.host_device = 1;
			if (i == phy->otg_phy_index) {
				reg0.b.IDPULLUP0 = 1;
				reg0.b.DRVVBUS0 = 1;
			}
		}
		writel(reg0.d32, u2p_aml_regs.u2p_r_v2[0]);
	}

	udelay(10);
	amlogic_new_c2_usbphy_reset_phycfg_v2(phy, phy->portnum);
	udelay(50);

	for (i = 0; i < phy->portnum; i++) {
		for (j = 0; j < 4; j++) {
			u2p_aml_regs.u2p_r_v2[j] = (void __iomem	*)
				((unsigned long)phy->regs + i * PHY_REGISTER_SIZE
				+ 4 * j);
		}

		usb_set_calibration_trim(phy->phy_cfg[i], phy);

		/* ID DETECT: usb2_otg_aca_en set to 0 */
		/* usb2_otg_iddet_en set to 1 */
		writel(readl(phy->phy_cfg[i] + 0x54) & (~(1 << 2)),
			(phy->phy_cfg[i] + 0x54));

		reg1.d32 = readl(u2p_aml_regs.u2p_r_v2[1]);
		cnt = 0;
		while (reg1.b.phy_rdy != 1) {
			reg1.d32 = readl(u2p_aml_regs.u2p_r_v2[1]);
			/*we wait phy ready max 1ms, common is 100us*/
			if (cnt > 200)
				break;

			cnt++;
			udelay(5);
		}
	}

	/* step 7: pll setting */
	for (i = 0; i < phy->portnum; i++)
		set_usb_pll(phy, phy->phy_cfg[i]);

	if (phy->suspend_flag)
		phy->suspend_flag = 0;

	return 0;
}

static int amlogic_new_usb2_suspend(struct usb_phy *x, int suspend)
{
	return 0;
}

static void amlogic_new_usb2phy_shutdown(struct usb_phy *x)
{
	struct amlogic_usb_v2 *phy = phy_to_amlusb(x);
	u32 val, i = 0;
	u32 temp = 0;
	u32 cnt = phy->portnum;
	size_t mask = 0;

	mask = (size_t)phy->reset_regs & 0xf;

	for (i = 0; i < cnt; i++)
		temp = temp | (1 << phy->phy_reset_level_bit[i]);

	/* set usb phy to low power mode */
	val = readl((void __iomem		*)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));
	writel((val & (~temp)), (void __iomem	*)
		((unsigned long)phy->reset_regs + (0x21 * 4 - mask)));

	if (phy->phy.flags == AML_USB2_PHY_ENABLE)
		clk_disable_unprepare(phy->clk);

	phy->suspend_flag = 1;
}

static int amlogic_new_c2_usb2_probe(struct platform_device *pdev)
{
	struct amlogic_usb_v2			*phy;
	struct device *dev = &pdev->dev;
	struct resource *phy_mem;
	struct resource *reset_mem;
	struct resource *phy_cfg_mem[4];
	void __iomem	*phy_base;
	void __iomem	*reset_base = NULL;
	void __iomem	*phy_cfg_base[4] = {NULL, NULL, NULL, NULL};
	unsigned int	usb_clk_reg;
	void __iomem	*usb_clk_reg_base = NULL;
	unsigned int	phy_trim_reg;
	void __iomem	*usb_phy_trim_reg = NULL;
	unsigned int clk_regsize = 0;
	int portnum = 0;
	int phy_version = 0;
	const void *prop;
	int i = 0;
	int retval;
	u32 pll_setting[8];
	u32 phy_reset_level_bit[USB_PHY_MAX_NUMBER] = {-1};
	u32 usb_reset_bit = 2;
	u32 otg_phy_index = 1;
	u32 val;
	u32 usbclk_div = 0;

	prop = of_get_property(dev->of_node, "portnum", NULL);
	if (prop)
		portnum = of_read_ulong(prop, 1);

	if (!portnum) {
		dev_err(&pdev->dev, "This phy has no usb port\n");
		return -ENOMEM;
	}

	prop = of_get_property(dev->of_node, "usb-busclk_ctl_div", NULL);
	if (prop) {
		usbclk_div = of_read_ulong(prop, 1);
		dev_dbg(&pdev->dev, "usb_clk_ctl_div=%u\n", usbclk_div);
	}

	prop = of_get_property(dev->of_node, "version", NULL);
	if (prop)
		phy_version = of_read_ulong(prop, 1);
	else
		phy_version = 0;

	prop = of_get_property(dev->of_node, "otg-phy-index", NULL);
	if (prop)
		otg_phy_index = of_read_ulong(prop, 1);
	else
		otg_phy_index = 1;

	phy_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	phy_base = devm_ioremap_resource(dev, phy_mem);
	if (IS_ERR(phy_base))
		return PTR_ERR(phy_base);

	reset_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (reset_mem) {
		reset_base = ioremap(reset_mem->start,
			resource_size(reset_mem));
		if (IS_ERR(reset_base))
			return PTR_ERR(reset_base);
	}

	for (i = 0; i < portnum; i++) {
		phy_cfg_mem[i] = platform_get_resource
					(pdev, IORESOURCE_MEM, 2 + i);
		if (phy_cfg_mem[i]) {
			phy_cfg_base[i] = ioremap(phy_cfg_mem[i]->start,
				resource_size(phy_cfg_mem[i]));
			if (IS_ERR(phy_cfg_base[i]))
				return PTR_ERR(phy_cfg_base[i]);
		}
	}

	retval = of_property_read_u32(dev->of_node, "usb-clk-reg",
				      &usb_clk_reg);
	if (retval >= 0) {
		retval = of_property_read_u32(dev->of_node,
					      "usb-clkreg-size",
					      &clk_regsize);
		if (retval >= 0) {
			usb_clk_reg_base = ioremap((resource_size_t)usb_clk_reg,
						   (unsigned long)clk_regsize);
			if (usb_clk_reg_base) {
				if (usbclk_div) {
					writel(usbclk_div, usb_clk_reg_base);
				} else {
					val = readl(usb_clk_reg_base);
					val |= (0x1 << 8) | (0x1 << 9)
						| (0 << 0);
					writel(val, usb_clk_reg_base);
				}
			}
		}
	}

	retval = of_property_read_u32(dev->of_node, "usb-phy-trim-reg",
				      &phy_trim_reg);
	if (retval >= 0) {
		usb_phy_trim_reg = ioremap((resource_size_t)phy_trim_reg,
					   4);
	}

	for (i = 0; i < portnum; i++) {
		memset(name_c, 0, 32 * sizeof(char));
		sprintf(name_c, "phy2%d-reset-level-bit", i);
		prop = of_get_property(dev->of_node, name_c, NULL);
		if (prop)
			phy_reset_level_bit[i] = of_read_ulong(prop, 1);
		else
			phy_reset_level_bit[i] = 16 + i;
	}

	prop = of_get_property(dev->of_node, "usb-reset-bit", NULL);
	if (prop)
		usb_reset_bit = of_read_ulong(prop, 1);
	else
		usb_reset_bit = 2;

	phy = devm_kzalloc(&pdev->dev, sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-1", &pll_setting[0]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-2", &pll_setting[1]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-3", &pll_setting[2]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-4", &pll_setting[3]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-5", &pll_setting[4]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
		"pll-setting-6", &pll_setting[5]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
			"pll-setting-7", &pll_setting[6]);
	if (retval < 0)
		return -EINVAL;

	retval = of_property_read_u32(dev->of_node,
			"pll-setting-8", &pll_setting[7]);
	if (retval < 0)
		return -EINVAL;

	dev_dbg(&pdev->dev, "USB2 phy probe:phy_mem:0x%lx, iomap phy_base:0x%lx\n",
			(unsigned long)phy_mem->start, (unsigned long)phy_base);

	phy->dev		= dev;
	phy->regs		= phy_base;
	phy->reset_regs = reset_base;
	phy->portnum      = portnum;
	phy->phy.dev		= phy->dev;
	phy->phy.label		= "amlogic-usbphy2";
	phy->phy.init		= amlogic_new_c2_usb2_init;
	phy->phy.set_suspend	= amlogic_new_usb2_suspend;
	phy->phy.shutdown	= amlogic_new_usb2phy_shutdown;
	phy->phy.type		= USB_PHY_TYPE_USB2;
	phy->pll_setting[0] = pll_setting[0];
	phy->pll_setting[1] = pll_setting[1];
	phy->pll_setting[2] = pll_setting[2];
	phy->pll_setting[3] = pll_setting[3];
	phy->pll_setting[4] = pll_setting[4];
	phy->pll_setting[5] = pll_setting[5];
	phy->pll_setting[6] = pll_setting[6];
	phy->pll_setting[7] = pll_setting[7];
	phy->suspend_flag = 0;
	phy->phy_version = phy_version;
	phy->otg_phy_index = otg_phy_index;
	phy->usb_phy_trim_reg = usb_phy_trim_reg;
	for (i = 0; i < portnum; i++) {
		phy->phy_cfg[i] = phy_cfg_base[i];
		/* set port default tuning state */
		phy->phy_cfg_state[i] = 1;
		phy->phy_reset_level_bit[i] = phy_reset_level_bit[i];
	}

	/**USB PHY CLOCK ENABLE**/
	phy->clk = devm_clk_get(dev, "usb_phy");
	if (!IS_ERR(phy->clk)) {
		retval = clk_prepare_enable(phy->clk);
		if (retval) {
			dev_err(dev, "Failed to enable usb2 phy bus clock\n");
			retval = PTR_ERR(phy->clk);
			return retval;
		}
		phy->phy.flags = AML_USB2_PHY_ENABLE;
	}

	phy->usb_reset_bit = usb_reset_bit;

	usb_add_phy_dev(&phy->phy);

	platform_set_drvdata(pdev, phy);

	pm_runtime_enable(phy->dev);

	g_c2_phy2 = phy;

	return 0;
}

static void amlogic_new_usb2_remove(struct platform_device *pdev)
{
}

#ifdef CONFIG_PM_RUNTIME

static int amlogic_new_usb2_runtime_suspend(struct device *dev)
{
	return 0;
}

static int amlogic_new_usb2_runtime_resume(struct device *dev)
{
	unsigned int ret = 0;

	return ret;
}

static const struct dev_pm_ops amlogic_new_c2_usb2_pm_ops = {
	SET_RUNTIME_PM_OPS(amlogic_new_usb2_runtime_suspend,
		amlogic_new_usb2_runtime_resume,
		NULL)
};

#define C2_DEV_PM_OPS     (&amlogic_new_c2_usb2_pm_ops)
#else
#define C2_DEV_PM_OPS     NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id amlogic_new_c2_usb2_id_table[] = {
	{ .compatible = "amlogic, amlogic-new-usb2-v2-c2" },
	{ .compatible = "amlogic,amlogic-new-usb2-v2-c2" },
	{}
};
MODULE_DEVICE_TABLE(of, amlogic_new_c2_usb2_id_table);
#endif

static struct platform_driver amlogic_new_c2_usb2_v2_driver = {
	.probe		= amlogic_new_c2_usb2_probe,
	.remove		= amlogic_new_usb2_remove,
	.driver		= {
		.name	= "amlogic-new-usb2-v2-c2",
		.owner	= THIS_MODULE,
		.pm	= C2_DEV_PM_OPS,
		.of_match_table = of_match_ptr(amlogic_new_c2_usb2_id_table),
	},
};

#if 0
module_platform_driver(amlogic_new_c2_usb2_v2_driver);

MODULE_ALIAS("platform: amlogic_usb2_c2");
MODULE_AUTHOR("Amlogic Inc.");
MODULE_DESCRIPTION("amlogic USB2 c2 phy driver");
MODULE_LICENSE("GPL v2");
#else
int __init amlogic_new_c2_usb2_v2_driver_init(void)
{
	return platform_driver_register(&amlogic_new_c2_usb2_v2_driver);
}
#endif


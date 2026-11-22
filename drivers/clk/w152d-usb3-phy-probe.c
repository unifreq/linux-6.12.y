// SPDX-License-Identifier: GPL-2.0-or-later
/* W152D USB3 warm boot. The analog sequence was verified on this board in U-Boot. */
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#define USB3_APB_RESET		BIT(13)
#define USB3_PHY_RESET		BIT(12)
#define USB3_CRG_RESET		BIT(4)
#define RESET_LEVEL		0x40
#define USB3_PLL_STATUS		0x154
#define USB3_PLL_LOCKED		(BIT(24) | BIT(25))
#define USB3_VBUS		BIT(11)
#define USB2_COMPANION_RESET	(BIT(4) | BIT(6) | BIT(8))
#define USB2_HOST_BITS		(BIT(0) | BIT(4) | BIT(5))
#define USB2_POR		BIT(3)
#define USB2_HUB_CRG_RESET	(BIT(5) | BIT(7) | BIT(9))
#define USB2_HUB_RESET_GPIO	BIT(6)
#define USB2_PORT_ENABLE_GPIOS	GENMASK(4, 2)

static DEFINE_SPINLOCK(w152d_usb_mmio_lock);

struct w152d_usb3_probe {
	struct device *dev;
	void __iomem *crg;
	struct delayed_work inspect_ports;
};

static void w152d_update_bits(void __iomem *addr, u32 mask, u32 value)
{
	unsigned long flags;

	spin_lock_irqsave(&w152d_usb_mmio_lock, flags);
	writel((readl(addr) & ~mask) | (value & mask), addr);
	spin_unlock_irqrestore(&w152d_usb_mmio_lock, flags);
}

static void w152d_usb3_inspect_ports(struct work_struct *work)
{
	struct w152d_usb3_probe *probe = container_of(to_delayed_work(work),
					struct w152d_usb3_probe, inspect_ports);
	void __iomem *op = probe->crg + (readl(probe->crg) & 0xff);

	dev_info(probe->dev, "USB3 controller PORTSC1=%#010x PORTSC2=%#010x\n",
		 readl(op + 0x400), readl(op + 0x410));
}

static void w152d_usb3_cancel_inspection(void *data)
{
	struct w152d_usb3_probe *probe = data;

	cancel_delayed_work_sync(&probe->inspect_ports);
}

static int w152d_usb3_start_phy(void __iomem *analog, void __iomem *reset, u32 *pll)
{
	int ret;

	w152d_update_bits(reset + RESET_LEVEL, USB3_APB_RESET | USB3_PHY_RESET,
			  USB3_APB_RESET | USB3_PHY_RESET);
	writel(USB3_APB_RESET, reset);
	w152d_update_bits(analog + 0x10, BIT(3), BIT(3));
	writel(0x07203234, analog + 0x2c);
	writel(0x52480000, analog + 0x34);
	writel(0x00010020, analog + 0x38);
	writel(USB3_PHY_RESET, reset);
	usleep_range(320, 400);
	ret = readl_poll_timeout(analog + USB3_PLL_STATUS, *pll,
				(*pll & USB3_PLL_LOCKED) == USB3_PLL_LOCKED,
				100, 5000);
	if (ret)
		return ret;

	writel(1, analog + 0x128);
	w152d_update_bits(analog + 0x50, GENMASK(31, 24), GENMASK(31, 24));
	w152d_update_bits(analog + 0x2c, BIT(3), BIT(3));
	w152d_update_bits(analog + 0x3c, BIT(0), BIT(0));
	usleep_range(200, 300);
	w152d_update_bits(analog + 0x3c, BIT(1), BIT(1));
	usleep_range(200, 300);
	w152d_update_bits(analog + 0x3c, BIT(2), BIT(2));
	usleep_range(200, 300);
	w152d_update_bits(analog + 0x2c, BIT(0), BIT(0));
	w152d_update_bits(analog + 0x2c, BIT(1), BIT(1));
	w152d_update_bits(analog + 0x2c, BIT(30), BIT(30));
	w152d_update_bits(analog + 0x30, BIT(0), BIT(0));

	w152d_update_bits(analog + 0xbc, BIT(14), BIT(14));
	w152d_update_bits(analog + 0xc0, BIT(28) | BIT(26), BIT(28) | BIT(26));
	w152d_update_bits(analog + 0xbc, BIT(17), BIT(17));
	w152d_update_bits(analog + 0xc, BIT(25) | BIT(23), BIT(23));
	w152d_update_bits(analog + 0xbc, BIT(0), BIT(0));
	w152d_update_bits(analog + 0xc0, BIT(0), BIT(0));
	w152d_update_bits(analog + 0xbc, BIT(1), BIT(1));
	w152d_update_bits(analog + 0xc0, BIT(2), BIT(2));
	writel(0x256814ac, analog + 0x10);
	writel(0xf8323183, analog + 0x4);
	w152d_update_bits(analog + 0x14, BIT(16), 0);
	w152d_update_bits(analog + 0x20, BIT(9), BIT(9));
	writel(0x0000c000, analog + 0x24);
	writel(USB3_CRG_RESET, reset);
	usleep_range(100, 200);
	*pll = readl(analog + USB3_PLL_STATUS);
	return 0;
}

static int w152d_usb3_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct w152d_usb3_probe *probe;
	struct resource *crg_res;
	void __iomem *analog, *reset, *gpio, *companion;
	struct clk *clk;
	u32 pll;
	int ret;

	analog = devm_platform_ioremap_resource_byname(pdev, "analog");
	if (IS_ERR(analog))
		return PTR_ERR(analog);
	reset = devm_platform_ioremap_resource_byname(pdev, "reset");
	if (IS_ERR(reset))
		return PTR_ERR(reset);
	gpio = devm_platform_ioremap_resource_byname(pdev, "vbus-gpio");
	if (IS_ERR(gpio))
		return PTR_ERR(gpio);
	companion = devm_platform_ioremap_resource_byname(pdev, "usb2-companion");
	if (IS_ERR(companion))
		return PTR_ERR(companion);
	crg_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "crg");
	if (!crg_res)
		return -EINVAL;
	probe = devm_kzalloc(dev, sizeof(*probe), GFP_KERNEL);
	if (!probe)
		return -ENOMEM;
	/* The generic xHCI driver owns this window. Only use the CRG glue bits. */
	probe->crg = devm_ioremap(dev, crg_res->start, resource_size(crg_res));
	if (!probe->crg)
		return -ENOMEM;
	probe->dev = dev;
	INIT_DELAYED_WORK(&probe->inspect_ports, w152d_usb3_inspect_ports);
	ret = devm_add_action_or_reset(dev, w152d_usb3_cancel_inspection, probe);
	if (ret)
		return ret;

	clk = devm_clk_get_enabled(dev, "crg");
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk), "no USB3 CRG clock\n");
	clk = devm_clk_get_enabled(dev, "pipe");
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk), "no USB3 PHY clock\n");

	/* No block device has been mounted in this diagnostic. */
	w152d_update_bits(gpio, USB3_VBUS, 0);
	w152d_update_bits(gpio + 4, USB3_VBUS, 0);
	msleep(100);
	ret = w152d_usb3_start_phy(analog, reset, &pll);
	w152d_update_bits(gpio, USB3_VBUS, USB3_VBUS);
	if (ret)
		return dev_err_probe(dev, ret, "USB3 PHY PLL failed to lock (%#x)\n", pll);
	w152d_update_bits(reset + RESET_LEVEL, USB2_COMPANION_RESET, USB2_COMPANION_RESET);
	w152d_update_bits(companion, USB2_POR | USB2_HOST_BITS, USB2_HOST_BITS);

	/* PHY reset precedes selecting host mode and SuperSpeed in the CRG glue. */
	w152d_update_bits(probe->crg + 0x20fc, BIT(0), 0);
	writel(4, probe->crg + 0x2410);

	dev_info(dev, "USB3 PHY PLL=%#x, companion=%#x/%#x, VBUS=%#x/%#x, host=%#x\n",
		 pll, readl(companion), readl(companion + 4), readl(gpio),
		 readl(gpio + 4), readl(probe->crg + 0x20fc));
	schedule_delayed_work(&probe->inspect_ports, msecs_to_jiffies(6000));
	return 0;
}

static const struct of_device_id w152d_usb3_phy_match[] = {
	{ .compatible = "amlogic,w152d-usb3-phy-probe" },
	{ }
};

static struct platform_driver w152d_usb3_phy_driver = {
	.probe = w152d_usb3_phy_probe,
	.driver = {
		.name = "w152d-usb3-phy-probe",
		.of_match_table = w152d_usb3_phy_match,
	},
};
builtin_platform_driver(w152d_usb3_phy_driver);

static void __iomem *w152d_usb_shared_regs(struct platform_device *pdev, const char *name)
{
	struct resource *res = platform_get_resource_byname(pdev, IORESOURCE_MEM, name);

	return res ? devm_ioremap(&pdev->dev, res->start, resource_size(res)) : NULL;
}

static int w152d_usb2_hub_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	void __iomem *phy, *h_gpio, *c_gpio, *reset, *crg;
	struct clk *clk;

	phy = devm_platform_ioremap_resource_byname(pdev, "phy");
	if (IS_ERR(phy))
		return PTR_ERR(phy);
	h_gpio = devm_platform_ioremap_resource_byname(pdev, "hub-reset-gpio");
	if (IS_ERR(h_gpio))
		return PTR_ERR(h_gpio);
	c_gpio = devm_platform_ioremap_resource_byname(pdev, "port-enable-gpio");
	if (IS_ERR(c_gpio))
		return PTR_ERR(c_gpio);
	reset = w152d_usb_shared_regs(pdev, "reset");
	crg = w152d_usb_shared_regs(pdev, "crg");
	if (!reset || !crg)
		return -ENODEV;
	clk = devm_clk_get_enabled(dev, NULL);
	if (IS_ERR(clk))
		return dev_err_probe(dev, PTR_ERR(clk), "no USB2 CRG clock\n");

	w152d_update_bits(reset + RESET_LEVEL, USB2_HUB_CRG_RESET, USB2_HUB_CRG_RESET);
	w152d_update_bits(phy, USB2_POR | USB2_HOST_BITS, USB2_HOST_BITS);
	w152d_update_bits(crg + 0x20fc, BIT(0), 0);

	/* The original kernel leaves GPIOH_6 low and GPIOC_2..4 high. */
	w152d_update_bits(c_gpio, USB2_PORT_ENABLE_GPIOS, USB2_PORT_ENABLE_GPIOS);
	w152d_update_bits(c_gpio + 4, USB2_PORT_ENABLE_GPIOS, 0);
	w152d_update_bits(h_gpio, USB2_HUB_RESET_GPIO, 0);
	w152d_update_bits(h_gpio + 4, USB2_HUB_RESET_GPIO, 0);
	dev_info(dev, "USB2 hub enabled, companion=%#x/%#x\n", readl(phy), readl(phy + 4));
	return 0;
}

static const struct of_device_id w152d_usb2_hub_match[] = {
	{ .compatible = "amlogic,w152d-usb2-hub-probe" },
	{ }
};

static struct platform_driver w152d_usb2_hub_driver = {
	.probe = w152d_usb2_hub_probe,
	.driver = {
		.name = "w152d-usb2-hub-probe",
		.of_match_table = w152d_usb2_hub_match,
	},
};
builtin_platform_driver(w152d_usb2_hub_driver);

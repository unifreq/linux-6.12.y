// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/amlogic/usb-v2.h>
#include <linux/workqueue.h>
#include "amlogic-cc.h"

struct cc_dev	*g_cc_dev;

void amlogic_cc_init(void)
{
	struct cc_dev *cc_dev = g_cc_dev;
	u32 device_host = 0;
	union u2p_r0_v2 u2p_reg0;
	union cc_r0 cc_reg0;

	if (g_cc_dev) {
		u2p_reg0.d32 = readl(cc_dev->phy_base);
		device_host = u2p_reg0.b.host_device;
		if (device_host == 0) {
			cc_reg0.d32 = readl(cc_dev->regs);
			cc_reg0.b.cc_en = 1;
			writel(cc_reg0.d32, (cc_dev->regs));
		} else {
			dev_info(cc_dev->dev,
				 "cc error, usb phy host mode!\n");
		}
	}
}
EXPORT_SYMBOL(amlogic_cc_init);

static void amlogic_cc_work(struct work_struct *work)
{
	struct cc_dev *cc_dev =
		container_of(work, struct cc_dev, work.work);
	struct device *dev = cc_dev->dev;
	union cc_r2 cc_reg2 = {.d32 = 0};
	union cc_r0 cc_reg0 = {.d32 = 0};
	u32 power_level = 0;
	u32 cc_plug_ab = 0;

	cc_reg2.d32 = cc_dev->cc_status;
	dev_info(dev, "cc STATUS is :");
	switch (cc_reg2.b.cc_int_status) {
	case 1:
		power_level = cc_reg2.b.cc_power_level;
		dev_info(dev, "power_level is 0x%x\n", power_level);
		break;
	case 2:
		dev_info(dev, "adapter plug out\n");
		break;
	case 4:
		power_level = cc_reg2.b.cc_power_level;
		cc_plug_ab = cc_reg2.b.cc_plug_ab;
		dev_info(dev, "adapter plug in\n");
		switch (power_level) {
		case 0:
			dev_info(dev, "power_level is not support\n");
			break;
		case 1:
			dev_info(dev, "power_level is 0.9A\n");
			break;
		case 2:
			dev_info(dev, "power_level is 1.5A\n");
			break;
		case 3:
			dev_info(dev, "power_level is 3A\n");
			break;
		}
		switch (cc_plug_ab) {
		case 0:
			dev_info(dev, "no plugin\n");
			break;
		case 1:
			dev_info(dev, "cc1 plugin\n");
			break;
		case 2:
			dev_info(dev, "cc2 plugin\n");
			break;
		case 3:
			dev_info(dev, "reserved\n");
			break;
		}
		break;
	case 8:
		dev_info(dev, "cc detect error\n");
		break;
	default:
		dev_info(dev, "RESERVED\n");
		break;
	}

	cc_reg0.d32 = readl(cc_dev->regs);
	cc_reg0.b.cc_en = 1;
	writel(cc_reg0.d32, cc_dev->regs);
}

static irqreturn_t amlogic_cc_detect_irq(int irq, void *_dev)
{
	struct cc_dev *cc_dev = (struct cc_dev *)_dev;
	union cc_r1 cc_reg1 = {.d32 = 0};

	cc_dev->cc_status = readl(cc_dev->regs + 0x8);

	cc_reg1.d32 = readl(cc_dev->regs + 0x4);
	cc_reg1.b.cc_int_clr = 1;
	writel(cc_reg1.d32, (cc_dev->regs + 0x4));

	schedule_delayed_work(&cc_dev->work, msecs_to_jiffies(5));
	return IRQ_HANDLED;
}

static int amlogic_cc_probe(struct platform_device *pdev)
{
	struct cc_dev	*cc_dev;
	struct device *dev = &pdev->dev;
	struct resource *cc_mem;
	struct resource *phy_mem;
	void __iomem	*cc_base;
	void __iomem *phy_base;
	int irq;
	int retval;

	cc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	cc_base = devm_ioremap_resource(dev, cc_mem);
	if (IS_ERR(cc_base))
		return PTR_ERR(cc_base);

	cc_dev = devm_kzalloc(&pdev->dev, sizeof(*cc_dev), GFP_KERNEL);
	if (!cc_dev)
		return -ENOMEM;

	phy_mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (phy_mem) {
		phy_base = ioremap(phy_mem->start,
				   resource_size(phy_mem));
		if (IS_ERR(phy_base))
			return PTR_ERR(phy_base);
	} else {
		return PTR_ERR(phy_mem);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return -ENODEV;
	retval = request_irq(irq, amlogic_cc_detect_irq,
			     IRQF_SHARED | IRQ_LEVEL,
			     "amlogic_cc_detect", cc_dev);

	if (retval) {
		dev_err(&pdev->dev, "request of irq%d failed\n",
			irq);
		retval = -EBUSY;
		return retval;
	}

	cc_dev->dev		= dev;
	cc_dev->regs	= cc_base;
	cc_dev->phy_base = phy_base;

	platform_set_drvdata(pdev, cc_dev);

	pm_runtime_enable(cc_dev->dev);

	INIT_DELAYED_WORK(&cc_dev->work, amlogic_cc_work);

	g_cc_dev = cc_dev;

	amlogic_cc_init();

	return 0;
}

static void amlogic_cc_remove(struct platform_device *pdev)
{
}

#ifdef CONFIG_PM_RUNTIME

static int amlogic_cc_runtime_suspend(struct device *dev)
{
	return 0;
}

static int amlogic_cc_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops amlogic_cc_pm_ops = {
	SET_RUNTIME_PM_OPS(amlogic_cc_runtime_suspend,
			   amlogic_cc_runtime_resume, NULL)
};

#define DEV_PM_OPS     (&amlogic_cc_pm_ops)
#else
#define DEV_PM_OPS     NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id amlogic_cc_id_table[] = {
	{ .compatible = "amlogic, cc" },
	{}
};
MODULE_DEVICE_TABLE(of, amlogic_cc_id_table);
#endif

static struct platform_driver amlogic_cc_driver = {
	.probe		= amlogic_cc_probe,
	.remove		= amlogic_cc_remove,
	.driver		= {
		.name	= "amlogic-cc",
		.owner	= THIS_MODULE,
		.pm	= DEV_PM_OPS,
		.of_match_table = of_match_ptr(amlogic_cc_id_table),
	},
};

#if 0
module_platform_driver(amlogic_cc_driver);

MODULE_ALIAS("platform: amlogic_cc");
MODULE_AUTHOR("Amlogic Inc.");
MODULE_DESCRIPTION("amlogic CC driver");
MODULE_LICENSE("GPL v2");
#else
int __init amlogic_cc_driver_init(void)
{
	return platform_driver_register(&amlogic_cc_driver);
}
#endif


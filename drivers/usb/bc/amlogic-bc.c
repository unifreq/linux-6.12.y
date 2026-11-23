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
#include "../usb_main.h"
#include "amlogic-bc.h"

struct bc_dev	*g_bc_dev;
struct usb_aml_regs_v2 g_usb_new_aml_regs_v2;

void amlogic_bc_init(void)
{
	struct bc_dev *bc_dev = g_bc_dev;
	u32 device_host = 0;
	union u2p_r0_v2 u2p_reg0;
	union bc_r0 bc_reg0;
	union usb_r0_v2 usb_r0;

	if (g_bc_dev) {
		u2p_reg0.d32 = readl(bc_dev->phy_base);
		device_host = u2p_reg0.b.host_device;
		if (device_host == 0) {
			/*cfg0_reg[31]*/
			usb_r0.d32 = readl(bc_dev->phy_base + 0x60);
			usb_r0.b.u2d_act = 1;
			writel(usb_r0.d32, bc_dev->phy_base + 0x60);
			mdelay(20);
			bc_reg0.d32 = readl(bc_dev->regs);
			bc_reg0.b.bc_en = 1;
			writel(bc_reg0.d32, (bc_dev->regs));
		} else {
			dev_info(bc_dev->dev,
				 "error, usb phy host mode!\n");
		}
	}
}
EXPORT_SYMBOL(amlogic_bc_init);

static void amlogic_bc_work(struct work_struct *work)
{
	struct bc_dev *bc_dev =
			container_of(work, struct bc_dev, work.work);
	struct device *dev = bc_dev->dev;
	union bc_r1 bc_reg1 = {.d32 = 0};
	union usb_r6_v2 r6 = {.d32 = 0};
	union u2p_r0_v2 reg0;

	reg0.d32 = readl(bc_dev->phy_base);
	if (reg0.b.IDPULLUP0) {
		r6.d32 = readl(g_usb_new_aml_regs_v2.usb_r_v2[6]);
		r6.b.usb_vbusdig_irq = 0;
		writel(r6.d32, g_usb_new_aml_regs_v2.usb_r_v2[6]);
		return;
	}

	bc_reg1.d32 = bc_dev->bc_status;
	dev_info(dev, "BC STATUS is :   ");
	switch (bc_reg1.b.port_status) {
	case 0:
		dev_info(dev, "default\n");
		break;
	case 1:
		dev_info(dev, "SDP\n");
		break;
	case 2:
		dev_info(dev, "DCP\n");
		break;
	case 3:
		dev_info(dev, "CDP\n");
		break;
	case 4:
		dev_info(dev, "ACA_A\n");
		break;
	case 5:
		dev_info(dev, "ACA_B\n");
		break;
	case 6:
		dev_info(dev, "ACA_C\n");
		break;
	case 7:
		dev_info(dev, "ACA_DOCK\n");
		break;
	case 8:
		dev_info(dev, "ACA GND ERROR\n");
		break;
	case 9:
		dev_info(dev, "analog output error\n");
		break;
	case 10:
		dev_info(dev, "VBUS remove\n");
		break;
	case 11:
		dev_info(dev, "VBUS invalid\n");
		break;
	default:
		dev_info(dev, "RESERVED\n");
		break;
	}

	udelay(9);

	r6.d32 = readl(g_usb_new_aml_regs_v2.usb_r_v2[6]);
	r6.b.usb_vbusdig_irq = 0;
	writel(r6.d32, g_usb_new_aml_regs_v2.usb_r_v2[6]);

	reg0.d32 = readl(bc_dev->phy_base);
	reg0.b.IDPULLUP0 = 1;
	writel(reg0.d32, bc_dev->phy_base);
}

static irqreturn_t amlogic_bc_detect_irq(int irq, void *_dev)
{
	struct bc_dev *bc_dev = (struct bc_dev *)_dev;
	union bc_r0 bc_reg0 = {.d32 = 0};

	bc_dev->bc_status = readl(bc_dev->regs + 0x4);

	bc_reg0.d32 = readl(bc_dev->regs);
	bc_reg0.b.bc_int_clean = 1;
	writel(bc_reg0.d32, (bc_dev->regs));
	schedule_delayed_work(&bc_dev->work, msecs_to_jiffies(10));

	return IRQ_HANDLED;
}

static void amlogic_vbus_work(struct work_struct *work)
{
	struct bc_dev *bc_dev =
		container_of(work, struct bc_dev, vbus_work.work);
	union bc_r0 bc_reg0 = {.d32 = 0};
	u32 device_host = 0;
	union u2p_r0_v2 u2p_reg0;

	u2p_reg0.d32 = readl(bc_dev->phy_base);
	device_host = u2p_reg0.b.host_device;
	if (device_host == 0) {
		amlogic_bc_init();
		u2p_reg0.b.IDPULLUP0 = 0;
		writel(u2p_reg0.d32, bc_dev->phy_base);

		bc_reg0.d32 = readl(bc_dev->regs);
		bc_reg0.b.bc_det_sw = 1;
		writel(bc_reg0.d32, (bc_dev->regs));
	}
}

static irqreturn_t amlogic_vbus_detect_irq(int irq, void *dev)
{
	struct bc_dev *bc_dev = (struct bc_dev *)dev;
	union usb_r6_v2 r6 = {.d32 = 0};

	r6.d32 = readl(g_usb_new_aml_regs_v2.usb_r_v2[6]);
	r6.b.usb_vbusdig_irq = 0;
	writel(r6.d32, g_usb_new_aml_regs_v2.usb_r_v2[6]);
	schedule_delayed_work(&bc_dev->vbus_work, msecs_to_jiffies(10));

	return IRQ_HANDLED;
}

static int amlogic_bc_probe(struct platform_device *pdev)
{
	struct bc_dev	*bc_dev;
	struct device *dev = &pdev->dev;
	struct resource *bc_mem;
	struct resource *phy_mem;
	void __iomem	*bc_base;
	void __iomem *phy_base;
	int irq;
	int retval;
	int i;

	bc_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	bc_base = devm_ioremap_resource(dev, bc_mem);
	if (IS_ERR(bc_base))
		return PTR_ERR(bc_base);

	bc_dev = devm_kzalloc(&pdev->dev, sizeof(*bc_dev), GFP_KERNEL);
	if (!bc_dev)
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

	retval = request_irq(irq, amlogic_bc_detect_irq,
			     IRQF_SHARED | IRQ_LEVEL,
			     "amlogic_bc_detect", bc_dev);

	if (retval) {
		dev_err(&pdev->dev, "request of irq%d failed\n",
			irq);
		retval = -EBUSY;
		return retval;
	}

	irq = platform_get_irq(pdev, 1);
	if (irq < 0)
		return -ENODEV;

	retval = request_irq(irq, amlogic_vbus_detect_irq,
			     IRQF_SHARED | IRQ_LEVEL,
			     "amlogic_vbus_detect", bc_dev);
	if (retval) {
		dev_err(&pdev->dev, "request of irq%d failed\n",
			irq);
		retval = -EBUSY;
		return retval;
	}

	bc_dev->dev		= dev;
	bc_dev->regs	= bc_base;
	bc_dev->phy_base = phy_base;

	for (i = 0; i < 7; i++) {
		g_usb_new_aml_regs_v2.usb_r_v2[i] = (void __iomem *)
			((unsigned long)phy_base + 0x60 + 4 * i);
	}

	platform_set_drvdata(pdev, bc_dev);

	pm_runtime_enable(bc_dev->dev);

	g_bc_dev = bc_dev;

	INIT_DELAYED_WORK(&bc_dev->work, amlogic_bc_work);
	INIT_DELAYED_WORK(&bc_dev->vbus_work, amlogic_vbus_work);

	return 0;
}

static void amlogic_bc_remove(struct platform_device *pdev)
{
}

#ifdef CONFIG_PM_RUNTIME

static int amlogic_bc_runtime_suspend(struct device *dev)
{
	return 0;
}

static int amlogic_bc_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops amlogic_bc_pm_ops = {
	SET_RUNTIME_PM_OPS(amlogic_bc_runtime_suspend,
			   amlogic_bc_runtime_resume,
			   NULL)
};

#define DEV_PM_OPS     (&amlogic_bc_pm_ops)
#else
#define DEV_PM_OPS     NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id amlogic_bc_id_table[] = {
	{ .compatible = "amlogic, bc" },
	{ .compatible = "amlogic,bc" },
	{}
};
MODULE_DEVICE_TABLE(of, amlogic_bc_id_table);
#endif

static struct platform_driver amlogic_bc_driver = {
	.probe		= amlogic_bc_probe,
	.remove		= amlogic_bc_remove,
	.driver		= {
		.name	= "amlogic-bc",
		.owner	= THIS_MODULE,
		.pm	= DEV_PM_OPS,
		.of_match_table = of_match_ptr(amlogic_bc_id_table),
	},
};

#if 0
module_platform_driver(amlogic_bc_driver);

MODULE_ALIAS("platform: amlogic_bc");
MODULE_AUTHOR("Amlogic Inc.");
MODULE_DESCRIPTION("amlogic BC driver");
MODULE_LICENSE("GPL v2");
#else
int __init amlogic_bc_driver_init(void)
{
	return platform_driver_register(&amlogic_bc_driver);
}
#endif


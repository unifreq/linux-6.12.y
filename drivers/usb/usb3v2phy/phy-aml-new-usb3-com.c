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
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <linux/usb/phy.h>
#include <linux/amlogic/usb-v2.h>
#include <linux/of_gpio.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/amlogic/usbtype.h>
#include "../phy/phy-aml-new-usb-v2.h"

struct amlogic_usb_v2	*g_phy_com_v2;

BLOCKING_NOTIFIER_HEAD(aml_new_usb_v2_notifier_list);

int aml_new_usb_v2_register_notifier(struct notifier_block *nb)
{
	int ret;

	ret = blocking_notifier_chain_register
			(&aml_new_usb_v2_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(aml_new_usb_v2_register_notifier);

int aml_new_usb_v2_unregister_notifier(struct notifier_block *nb)
{
	int ret;

	ret = blocking_notifier_chain_unregister
			(&aml_new_usb_v2_notifier_list, nb);

	return ret;
}
EXPORT_SYMBOL(aml_new_usb_v2_unregister_notifier);

void aml_new_usb3_get_phy(struct amlogic_usb_v2 *phy)
{
	g_phy_com_v2 = phy;
}
EXPORT_SYMBOL(aml_new_usb3_get_phy);

void aml_new_usb_notifier_call(unsigned long is_device_on)
{
	blocking_notifier_call_chain
			(&aml_new_usb_v2_notifier_list, is_device_on, NULL);
}
EXPORT_SYMBOL(aml_new_usb_notifier_call);

void resume_xhci_port_a(void)
{
	if (!g_phy_com_v2)
		return;
	if (g_phy_com_v2->resume_xhci_p_a)
		g_phy_com_v2->resume_xhci_p_a();
}
EXPORT_SYMBOL(resume_xhci_port_a);

void force_disable_xhci_port_a(void)
{
	if (!g_phy_com_v2)
		return;
	if (g_phy_com_v2->disable_port_a)
		g_phy_com_v2->disable_port_a();
}
EXPORT_SYMBOL(force_disable_xhci_port_a);

void aml_new_usb_v2_init(void)
{
	if (!g_phy_com_v2)
		return;
	if (g_phy_com_v2->usb2_phy_init)
		g_phy_com_v2->usb2_phy_init();
}
EXPORT_SYMBOL(aml_new_usb_v2_init);

int aml_new_usb_get_mode(void)
{
	if (!g_phy_com_v2)
		return -1;
	if (g_phy_com_v2->usb2_get_mode)
		return g_phy_com_v2->usb2_get_mode();
	return -1;
}
EXPORT_SYMBOL(aml_new_usb_get_mode);

void cr_bus_addr(unsigned int addr)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	phy_r4.b.phy_cr_data_in = addr;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_addr = 1;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_addr = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}
EXPORT_SYMBOL(cr_bus_addr);

int cr_bus_read(unsigned int addr)
{
	int data;
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	cr_bus_addr(addr);

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	phy_r4.b.phy_cr_read = 1;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);

	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	data = phy_r5.b.phy_cr_data_out;

	phy_r4.b.phy_cr_read = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
			msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	return data;
}
EXPORT_SYMBOL(cr_bus_read);

void cr_bus_write(unsigned int addr, unsigned int data)
{
	union phy3_r4 phy_r4 = {.d32 = 0};
	union phy3_r5 phy_r5 = {.d32 = 0};
	unsigned long timeout_jiffies;

	cr_bus_addr(addr);

	phy_r4.b.phy_cr_data_in = data;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	phy_r4.b.phy_cr_cap_data = 1;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_cap_data = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	phy_r4.b.phy_cr_write = 1;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 0 &&
		time_is_after_jiffies(timeout_jiffies));

	phy_r4.b.phy_cr_write = 0;
	writel(phy_r4.d32, g_phy_com_v2->phy3_cfg_r4);
	timeout_jiffies = jiffies +
		msecs_to_jiffies(1000);
	do {
		phy_r5.d32 = readl(g_phy_com_v2->phy3_cfg_r5);
	} while (phy_r5.b.phy_cr_ack == 1 &&
		time_is_after_jiffies(timeout_jiffies));
}
EXPORT_SYMBOL(cr_bus_write);

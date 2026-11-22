/*
 * Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/gpio.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(7, 1, 0)
#include <linux/gpio/consumer.h>
#else
#include <linux/of_gpio.h>
#endif
#include <marlin_platform.h>
#include "rfkill.h"
#include "bt_init.h"


static struct rfkill *bt_rfk;
static struct device *bt_dev;
static const char bt_name[] = "bluetooth";

static int bluetooth_set_power(void *data, bool blocked)
{
	int ret;

	pr_info("%s: start_block=%d\n", __func__, blocked);
	if (!blocked) {
		ret = start_marlin(MARLIN_BLUETOOTH);
		if (ret) {
			pr_err("%s: start_marlin failed: %d\n", __func__, ret);
			return ret;
		}
		/*
		 * The core is powered but unconfigured; send PSKEY/RF/enable
		 * so that a plain HCI line discipline can drive it. On failure
		 * power it down again and report the error, so the rfkill
		 * switch stays blocked and userspace can retry.
		 */
		ret = sprd_bt_init(bt_dev);
		if (ret) {
			pr_err("%s: Bluetooth vendor init failed: %d\n",
				__func__, ret);
			stop_marlin(MARLIN_BLUETOOTH);
			return ret;
		}
	} else {
		sprd_bt_init_invalidate();
		ret = stop_marlin(MARLIN_BLUETOOTH);
		if (ret)
			pr_err("%s: stop_marlin failed: %d\n", __func__, ret);
	}

	pr_info("%s: end_block=%d\n", __func__, blocked);
	return 0;
}

static struct rfkill_ops rfkill_bluetooth_ops = {
	.set_block = bluetooth_set_power,
};

int rfkill_bluetooth_init(struct platform_device *pdev)
{

	int rc = 0;

	pr_info("-->%s\n", __func__);
	bt_dev = &pdev->dev;
	bt_rfk = rfkill_alloc(bt_name, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
			&rfkill_bluetooth_ops, NULL);
	if (!bt_rfk) {
		rc = -ENOMEM;
		goto err_rfkill_alloc;
	}
	/* userspace cannot take exclusive control */
	rfkill_init_sw_state(bt_rfk, true);
	rc = rfkill_register(bt_rfk);
	if (rc)
		goto err_rfkill_reg;

	pr_info("<--%s\n", __func__);

	return 0;

err_rfkill_reg:
	rfkill_destroy(bt_rfk);
	bt_rfk = NULL;
err_rfkill_alloc:
	return rc;
}

int rfkill_bluetooth_remove(struct platform_device *dev)
{
	pr_info("-->%s\n", __func__);
	if (bt_rfk) {
		rfkill_unregister(bt_rfk);
		/*
		 * Unregistering does not call set_block, so power Bluetooth
		 * down here. Otherwise its power bit stays set and the chip
		 * is never powered off once WiFi is down too.
		 */
		if (!rfkill_blocked(bt_rfk))
			stop_marlin(MARLIN_BLUETOOTH);
		rfkill_destroy(bt_rfk);
		bt_rfk = NULL;
	}
	pr_info("<--%s\n", __func__);
	return 0;
}



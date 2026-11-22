/* SPDX-License-Identifier: GPL-2.0 */
/*
 * In-kernel Bluetooth vendor initialisation for the Unisoc UWE5622
 * (PSKEY / RF parameters / core enable), replacing the closed hciattach_opi.
 */
#ifndef _SPRD_BT_INIT_H_
#define _SPRD_BT_INIT_H_

#include <linux/device.h>

#if IS_ENABLED(CONFIG_TTY_OVERY_SDIO_BT_VENDOR_INIT)
bool sprd_bt_vendor_init_enabled(void);
int sprd_bt_init(struct device *dev);
void sprd_bt_init_invalidate(void);
#else
static inline bool sprd_bt_vendor_init_enabled(void) { return false; }
static inline int sprd_bt_init(struct device *dev) { return 0; }
static inline void sprd_bt_init_invalidate(void) { }
#endif

#endif

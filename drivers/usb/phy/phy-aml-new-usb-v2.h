/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/usb/phy.h>
#include <linux/amlogic/usb-v2.h>

#define	phy_to_amlusb(x)	container_of((x), struct amlogic_usb_v2, phy)
#define TUNING_DISCONNECT_THRESHOLD 0x3f

void aml_new_usb3_get_phy(struct amlogic_usb_v2 *phy);
void aml_new_usb_v2_init(void);
void set_usb_phy_host_tuning(int port, int default_val);
void set_usb_phy_device_tuning(int port, int default_val);

int amlogic_new_usbphy_reset_v2(struct amlogic_usb_v2 *phy);
int amlogic_new_usbphy_reset_phycfg_v2
	(struct amlogic_usb_v2 *phy, int cnt);
int amlogic_crg_drd_usbphy_reset(struct amlogic_usb_v2 *phy);
int amlogic_crg_drd_usbphy_reset_phycfg
	(struct amlogic_usb_v2 *phy, int cnt);
void crg_exit(void);
int crg_init(void);
void crg_gadget_exit(void);
int crg_gadget_init(void);
int crg_otg_write_UDC(const char *udc_name);


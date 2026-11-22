// SPDX-License-Identifier: GPL-2.0
/*
 * In-kernel Bluetooth vendor initialisation for the Unisoc UWE5622.
 *
 * The BT core of the UWE5622 comes up with no PSKEY/RF configuration and
 * disabled. The vendor tool (hciattach_opi, closed source) reads
 * bt_configure_pskey.ini and bt_configure_rf.ini, serialises them and sends
 * three vendor HCI commands before handing the tty to the HCI line
 * discipline:
 *
 *   0xFCA0  PSKEY          (176 bytes)
 *   0xFCA2  RF parameters  (252 bytes)
 *   0xFCA1  core enable    (00 00 01)
 *
 * Do the same here when the rfkill switch powers the BT core on, so that
 * stock BlueZ (btattach -B /dev/ttyBT0 -P h4) is all userspace needs.
 *
 * Based on the OpenWrt port by Reinerost
 * (https://github.com/Reinerost/UWE5622-OpenWrt-Driver, tty-sdio/bt_init.c,
 * GPL-2.0), tested on a WalnutPi 1B (H618 + UWE5622). Changes for
 * armbian/uwe5622: BD address from the bdaddr= module parameter or the
 * Allwinner addr_mgt driver (as unisocwifi does for the WLAN MAC), with the
 * ini value as last resort; vendor_init= switch to keep the blob usable;
 * one-shot latch cleared when the core is powered down or the firmware
 * resets.
 */

#include <linux/device.h>
#include <linux/errno.h>
#include <linux/firmware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/types.h>

#include "bt_init.h"
#include "woble.h"

#define HCI_PSKEY		0xFCA0
#define HCI_CORE_ENABLE		0xFCA1
#define HCI_RF_PARA		0xFCA2

#define BT_PSKEY_FILE		"bt_configure_pskey.ini"
#define BT_RF_FILE		"bt_configure_rf.ini"
/* armbian-firmware ships them in the root and under uwe5622/ */
#define BT_FW_SUBDIR		"uwe5622/"

#define BT_PSKEY_LEN		176
#define BT_RF_LEN		252
#define BD_ADDR_LEN		6

#define MAX_INI_ENTRIES		64
#define MAX_INI_VALUES		16

static bool vendor_init = true;
module_param(vendor_init, bool, 0444);
MODULE_PARM_DESC(vendor_init,
	"Send PSKEY/RF/enable to the BT core when rfkill powers it on (default Y); N keeps the controller raw for the vendor hciattach");

static char *bdaddr;
module_param(bdaddr, charp, 0444);
MODULE_PARM_DESC(bdaddr,
	"Bluetooth device address XX:XX:XX:XX:XX:XX (default: Allwinner addr_mgt \"bt\" address, else the one in bt_configure_pskey.ini)");

#if IS_ENABLED(CONFIG_SUNXI_ADDR_MGT)
extern int get_custom_mac_address(int fmt, char *name, char *addr);
#endif

struct ini_entry {
	char key[64];
	u32 value[MAX_INI_VALUES];
	size_t count;
};

struct ini_config {
	struct ini_entry entry[MAX_INI_ENTRIES];
	size_t count;
};

/* Serialised state: init runs from the rfkill set_block callback. */
static DEFINE_MUTEX(sprd_bt_init_lock);
static bool sprd_bt_initialized;

bool sprd_bt_vendor_init_enabled(void)
{
	return vendor_init;
}

void sprd_bt_init_invalidate(void)
{
	mutex_lock(&sprd_bt_init_lock);
	sprd_bt_initialized = false;
	mutex_unlock(&sprd_bt_init_lock);
}

static char *trim(char *s)
{
	char *e;

	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
		s++;

	if (!*s)
		return s;

	e = s + strlen(s) - 1;
	while (e >= s && (*e == ' ' || *e == '\t' || *e == '\r' || *e == '\n'))
		*e-- = '\0';

	return s;
}

/*
 * The ini files are "key = v1, v2, ..." lines, '#' comments, decimal or
 * 0x-prefixed values. Unknown keys are kept and simply never looked up.
 */
static int parse_ini(const struct firmware *fw, struct ini_config *cfg)
{
	char *buf, *cur, *line;
	int ret = 0;

	buf = kmemdup_nul(fw->data, fw->size, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	cur = buf;
	while ((line = strsep(&cur, "\n")) != NULL) {
		struct ini_entry *entry;
		char *s, *eq, *values;

		s = trim(line);
		if (!*s || *s == '#')
			continue;

		eq = strchr(s, '=');
		if (!eq) {
			ret = -EINVAL;
			goto out;
		}
		*eq++ = '\0';

		s = trim(s);
		values = trim(eq);
		if (!*s || !*values || cfg->count >= MAX_INI_ENTRIES ||
		    strlen(s) >= sizeof(cfg->entry[0].key)) {
			ret = -EINVAL;
			goto out;
		}

		entry = &cfg->entry[cfg->count++];
		strscpy(entry->key, s, sizeof(entry->key));

		while (*values) {
			char *comma, *value;
			u32 v;

			if (entry->count >= MAX_INI_VALUES) {
				ret = -E2BIG;
				goto out;
			}

			comma = strchr(values, ',');
			if (comma)
				*comma = '\0';

			value = trim(values);
			ret = kstrtou32(value, 0, &v);
			if (ret)
				goto out;

			entry->value[entry->count++] = v;

			if (!comma)
				break;
			values = trim(comma + 1);
		}

		if (!entry->count) {
			ret = -EINVAL;
			goto out;
		}
	}

out:
	kfree(buf);
	return ret;
}

static const struct ini_entry *ini_get(const struct ini_config *cfg,
				       const char *key, size_t min_count,
				       size_t max_count)
{
	size_t i;

	for (i = 0; i < cfg->count; i++) {
		const struct ini_entry *entry = &cfg->entry[i];

		if (strcmp(entry->key, key))
			continue;
		if (entry->count < min_count || entry->count > max_count)
			return NULL;
		return entry;
	}

	return NULL;
}

static void put_u8(u8 **p, u8 v)
{
	*(*p)++ = v;
}

static void put_u16(u8 **p, u16 v)
{
	*(*p)++ = v & 0xff;
	*(*p)++ = (v >> 8) & 0xff;
}

static void put_u32(u8 **p, u32 v)
{
	*(*p)++ = v & 0xff;
	*(*p)++ = (v >> 8) & 0xff;
	*(*p)++ = (v >> 16) & 0xff;
	*(*p)++ = (v >> 24) & 0xff;
}

static int put_ini8(u8 **p, const struct ini_config *cfg, const char *key,
		    size_t n)
{
	const struct ini_entry *entry = ini_get(cfg, key, n, n);
	size_t i;

	if (!entry)
		return -EINVAL;
	for (i = 0; i < n; i++) {
		if (entry->value[i] > U8_MAX)
			return -ERANGE;
		put_u8(p, entry->value[i]);
	}
	return 0;
}

static int put_ini16(u8 **p, const struct ini_config *cfg, const char *key,
		     size_t n)
{
	const struct ini_entry *entry = ini_get(cfg, key, n, n);
	size_t i;

	if (!entry)
		return -EINVAL;
	for (i = 0; i < n; i++) {
		if (entry->value[i] > U16_MAX)
			return -ERANGE;
		put_u16(p, entry->value[i]);
	}
	return 0;
}

static int put_ini32(u8 **p, const struct ini_config *cfg, const char *key,
		     size_t min_n, size_t max_n)
{
	const struct ini_entry *entry = ini_get(cfg, key, min_n, max_n);
	size_t i;

	if (!entry)
		return -EINVAL;
	for (i = 0; i < entry->count; i++)
		put_u32(p, entry->value[i]);
	return entry->count;
}

/*
 * Pick the BD address, in the human (most significant byte first) order:
 * 1. bdaddr= module parameter
 * 2. Allwinner addr_mgt "bt" address (derived from the SoC chip id, the
 *    same source hciattach_opi reads through /sys/class/addr_mgt/addr_bt)
 * 3. device_addr from the ini (a fixed per-image default, last resort)
 */
static int sprd_bt_get_bdaddr(struct device *dev, const struct ini_config *cfg,
			      u8 addr[BD_ADDR_LEN])
{
	const struct ini_entry *entry;
	size_t i;

	if (bdaddr && *bdaddr) {
		if (sscanf(bdaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			   &addr[0], &addr[1], &addr[2],
			   &addr[3], &addr[4], &addr[5]) == BD_ADDR_LEN) {
			dev_info(dev, "BD address %pM from bdaddr=\n", addr);
			return 0;
		}
		dev_warn(dev, "ignoring malformed bdaddr=\"%s\"\n", bdaddr);
	}

#if IS_ENABLED(CONFIG_SUNXI_ADDR_MGT)
	/* fmt 1 = raw bytes; returns the address type (>= 0) on success */
	if (get_custom_mac_address(1, "bt", (char *)addr) >= 0) {
		dev_info(dev, "BD address %pM from addr_mgt\n", addr);
		return 0;
	}
	dev_warn(dev, "addr_mgt has no bt address, using the ini default\n");
#endif

	entry = ini_get(cfg, "device_addr", BD_ADDR_LEN, BD_ADDR_LEN);
	if (!entry)
		return -EINVAL;
	for (i = 0; i < BD_ADDR_LEN; i++) {
		if (entry->value[i] > U8_MAX)
			return -ERANGE;
		addr[i] = entry->value[i];
	}
	dev_info(dev, "BD address %pM from %s\n", addr, BT_PSKEY_FILE);
	return 0;
}

static size_t build_pskey(struct device *dev, u8 *buf,
			  const struct ini_config *cfg)
{
	u8 addr[BD_ADDR_LEN];
	u8 *p = buf;
	int n, i;

#define P8(k, n)	do { if (put_ini8(&p, cfg, k, n) < 0) goto bad; } while (0)
#define P16(k, n)	do { if (put_ini16(&p, cfg, k, n) < 0) goto bad; } while (0)
#define P32(k, n)	do { if (put_ini32(&p, cfg, k, n, n) < 0) goto bad; } while (0)

	P32("device_class", 1);
	P8("feature_set", 16);

	/*
	 * device_addr: BD_ADDR as the controller stores it, least significant
	 * byte first (HCI order), i.e. the reverse of the printed form.
	 */
	if (sprd_bt_get_bdaddr(dev, cfg, addr))
		goto bad;
	for (i = BD_ADDR_LEN - 1; i >= 0; i--)
		put_u8(&p, addr[i]);

	P16("comp_id", 1);
	P8("g_sys_uart0_communication_supported", 1);
	P8("cp2_log_mode", 1);
	P8("LogLevel", 1);
	P8("g_central_or_perpheral", 1);
	P16("Log_BitMask", 1);
	P8("super_ssp_enable", 1);
	P8("common_rfu_b3", 1);
	P32("common_rfu_w", 2);
	P32("le_rfu_w", 2);
	P32("lmp_rfu_w", 2);
	P32("lc_rfu_w", 2);
	P16("g_wbs_nv_117", 1);
	P16("g_wbs_nv_118", 1);
	P16("g_nbv_nv_117", 1);
	P16("g_nbv_nv_118", 1);
	P8("g_sys_sco_transmit_mode", 1);
	P8("audio_rfu_b1", 1);
	P8("audio_rfu_b2", 1);
	P8("audio_rfu_b3", 1);
	P32("audio_rfu_w", 2);
	P8("g_sys_sleep_in_standby_supported", 1);
	P8("g_sys_sleep_master_supported", 1);
	P8("g_sys_sleep_slave_supported", 1);
	P8("power_rfu_b1", 1);
	P32("power_rfu_w", 2);
	P32("win_ext", 1);
	P8("edr_tx_edr_delay", 1);
	P8("edr_rx_edr_delay", 1);
	P8("tx_delay", 1);
	P8("rx_delay", 1);
	P32("bb_rfu_w", 2);
	P8("agc_mode", 1);
	P8("diff_or_eq", 1);
	P8("ramp_mode", 1);
	P8("modem_rfu_b1", 1);
	P32("modem_rfu_w", 2);
	P32("BQB_BitMask_1", 1);
	P32("BQB_BitMask_2", 1);
	P16("bt_coex_threshold", 8);

	/*
	 * The ini carries 2 other_rfu_w words (160-byte table); the vendor
	 * tool sends a 176-byte PSKEY, zero-filling up to 6 words.
	 */
	n = put_ini32(&p, cfg, "other_rfu_w", 2, 6);
	if (n < 0)
		goto bad;
	while (n++ < 6)
		put_u32(&p, 0);

#undef P8
#undef P16
#undef P32
	return p - buf;
bad:
	return 0;
}

static size_t build_rf(u8 *buf, const struct ini_config *cfg)
{
	u8 *p = buf;

#define R8(k, n)	do { if (put_ini8(&p, cfg, k, n) < 0) return 0; } while (0)
#define R16(k, n)	do { if (put_ini16(&p, cfg, k, n) < 0) return 0; } while (0)
#define R32(k, n)	do { if (put_ini32(&p, cfg, k, n, n) < 0) return 0; } while (0)

	R16("g_GainValue_A", 6);
	R16("g_ClassicPowerValue_A", 10);
	R16("g_LEPowerValue_A", 16);
	R16("g_BRChannelpwrvalue_A", 8);
	R16("g_EDRChannelpwrvalue_A", 8);
	R16("g_LEChannelpwrvalue_A", 8);

	R16("g_GainValue_B", 6);
	R16("g_ClassicPowerValue_B", 10);
	R16("g_LEPowerValue_B", 16);
	R16("g_BRChannelpwrvalue_B", 8);
	R16("g_EDRChannelpwrvalue_B", 8);
	R16("g_LEChannelpwrvalue_B", 8);

	R16("LE_fix_powerword", 1);
	R8("Classic_pc_by_channel", 1);
	R8("LE_pc_by_channel", 1);
	R8("RF_switch_mode", 1);
	R8("Data_Capture_Mode", 1);
	R8("Analog_IQ_Debug_Mode", 1);
	R8("RF_common_rfu_b3", 1);
	R32("RF_common_rfu_w", 5);

#undef R8
#undef R16
#undef R32
	return p - buf;
}

static int send_vendor_cmd(u16 opcode, const u8 *data, size_t len)
{
	struct HC_BT_HDR *payload;
	int ret;

	if (len > U8_MAX)
		return -EINVAL;

	payload = kzalloc(sizeof(*payload) + len, GFP_KERNEL);
	if (!payload)
		return -ENOMEM;

	payload->len = len;
	memcpy(payload->data, data, len);

	ret = hci_cmd_send_sync(opcode, payload, NULL);

	kfree(payload);
	return ret;
}

static int load_config(struct device *dev, const char *name,
		       struct ini_config *cfg)
{
	const struct firmware *fw;
	char subdir_name[64];
	int ret;

	ret = request_firmware(&fw, name, dev);
	if (ret) {
		snprintf(subdir_name, sizeof(subdir_name), "%s%s",
			 BT_FW_SUBDIR, name);
		ret = request_firmware(&fw, subdir_name, dev);
	}
	if (ret) {
		dev_err(dev, "failed to load %s: %d\n", name, ret);
		return ret;
	}

	memset(cfg, 0, sizeof(*cfg));
	ret = parse_ini(fw, cfg);
	release_firmware(fw);
	if (ret)
		dev_err(dev, "failed to parse %s: %d\n", name, ret);

	return ret;
}

static int __sprd_bt_init(struct device *dev)
{
	struct ini_config *cfg;
	u8 *payload;
	size_t len;
	int ret;

	cfg = kzalloc(sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	payload = kzalloc(BT_RF_LEN, GFP_KERNEL);
	if (!payload) {
		ret = -ENOMEM;
		goto out_cfg;
	}

	ret = load_config(dev, BT_PSKEY_FILE, cfg);
	if (ret)
		goto out;

	len = build_pskey(dev, payload, cfg);
	if (len != BT_PSKEY_LEN) {
		dev_err(dev, "%s: incomplete or malformed, PSKEY length %zu\n",
			BT_PSKEY_FILE, len);
		ret = -EINVAL;
		goto out;
	}

	ret = send_vendor_cmd(HCI_PSKEY, payload, len);
	if (ret) {
		dev_err(dev, "PSKEY (0x%04x) failed: %d\n", HCI_PSKEY, ret);
		goto out;
	}

	memset(payload, 0, BT_RF_LEN);
	ret = load_config(dev, BT_RF_FILE, cfg);
	if (ret)
		goto out;

	len = build_rf(payload, cfg);
	if (len != BT_RF_LEN) {
		dev_err(dev, "%s: incomplete or malformed, RF length %zu\n",
			BT_RF_FILE, len);
		ret = -EINVAL;
		goto out;
	}

	ret = send_vendor_cmd(HCI_RF_PARA, payload, len);
	if (ret) {
		dev_err(dev, "RF parameters (0x%04x) failed: %d\n",
			HCI_RF_PARA, ret);
		goto out;
	}

	payload[0] = 0x00;
	payload[1] = 0x00;
	payload[2] = 0x01;
	ret = send_vendor_cmd(HCI_CORE_ENABLE, payload, 3);
	if (ret) {
		dev_err(dev, "core enable (0x%04x) failed: %d\n",
			HCI_CORE_ENABLE, ret);
		goto out;
	}

	dev_info(dev, "Bluetooth vendor init done (PSKEY %u B, RF %u B)\n",
		 BT_PSKEY_LEN, BT_RF_LEN);
out:
	kfree(payload);
out_cfg:
	kfree(cfg);
	return ret;
}

/*
 * Called from the rfkill set_block(unblocked) path after start_marlin()
 * powered the BT core. Idempotent: "rfkill unblock" may be repeated
 * (systemd-rfkill restore, then the attach service) and the firmware
 * must only be configured once per power-up.
 */
int sprd_bt_init(struct device *dev)
{
	int ret = 0;

	if (!vendor_init)
		return 0;

	mutex_lock(&sprd_bt_init_lock);
	if (!sprd_bt_initialized) {
		ret = __sprd_bt_init(dev);
		if (!ret)
			sprd_bt_initialized = true;
	}
	mutex_unlock(&sprd_bt_init_lock);

	return ret;
}

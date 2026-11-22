// SPDX-License-Identifier: GPL-2.0
/* W152D mainline: board-level hooks that stock provides via wifi_comm.ko
 * (amlogic SDIO common driver). This device's wifi has no GPIO power
 * control (stock probe log: parse_dts=0, all gpio=-1), so empty
 * implementations are sufficient; the chip powers on via the software
 * sequence in sprdwl_probe. irq_num=0 keeps the in-band IRQ path
 * (stock log: sdio_irq_type=1). */
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>

void extern_wifi_set_enable(int is_on)
{
	struct device_node *node;
	struct regmap *gpio;
	int ret;

	node = of_find_compatible_node(NULL, NULL, "amlogic,w152d-wifi-gpio");
	if (!node)
		return;
	gpio = syscon_node_to_regmap(node);
	of_node_put(node);
	if (IS_ERR(gpio)) {
		pr_err("W152D: cannot map Wi-Fi power GPIO: %ld\n", PTR_ERR(gpio));
		return;
	}

	/* Bank X's output and active-low output-enable registers. */
	ret = regmap_update_bits(gpio, 4, BIT(6), is_on ? BIT(6) : 0);
	if (!ret)
		ret = regmap_update_bits(gpio, 8, BIT(6), 0);
	if (ret) {
		pr_err("W152D: cannot set GPIOX_6: %d\n", ret);
		return;
	}
	pr_info("W152D: GPIOX_6 Wi-Fi power=%d\n", !!is_on);
	if (is_on)
		msleep(180);
}
EXPORT_SYMBOL_GPL(extern_wifi_set_enable);

void extern_bt_set_enable(int is_on) { }
EXPORT_SYMBOL_GPL(extern_bt_set_enable);

void wifi_irq_trigger_level(int level) { }
EXPORT_SYMBOL_GPL(wifi_irq_trigger_level);

int wifi_irq_num(void) { return 0; }
EXPORT_SYMBOL_GPL(wifi_irq_num);

void sdio_set_max_reqsz(unsigned int sz) { }
EXPORT_SYMBOL_GPL(sdio_set_max_reqsz);

void sdio_clk_always_on(int on) { }

void sdio_reinit(void) { }
EXPORT_SYMBOL_GPL(sdio_reinit);
EXPORT_SYMBOL_GPL(sdio_clk_always_on);

/* cpu-version stubs: BSP defines these in drivers/amlogic/media; on GXLX4
 * the chip is MESON_CPU_MAJOR_ID_GXLX (S905L4). */
#include <linux/amlogic/media/registers/cpu_version.h>
unsigned char get_meson_cpu_version(int level)
{
	return (unsigned char)MESON_CPU_MAJOR_ID_GXLX;
}
EXPORT_SYMBOL_GPL(get_meson_cpu_version);

int get_cpu_type_from_media(void)
{
	return MESON_CPU_MAJOR_ID_GXLX;
}
EXPORT_SYMBOL_GPL(get_cpu_type_from_media);

/* USB gadget-side hooks (crg_udc removed from build; host mode only) */
void crg_gadget_init(void) { }
EXPORT_SYMBOL_GPL(crg_gadget_init);
void crg_gadget_exit(void) { }
EXPORT_SYMBOL_GPL(crg_gadget_exit);
void crg_otg_write_UDC(int mode) { }
EXPORT_SYMBOL_GPL(crg_otg_write_UDC);

/* usb3 phy helper (phy-aml-new-usb3-com.c platform hook) */

int aml_dwc3_init(void) { return 0; }
EXPORT_SYMBOL_GPL(aml_dwc3_init);

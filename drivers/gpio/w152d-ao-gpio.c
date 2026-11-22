// SPDX-License-Identifier: GPL-2.0-only
#include <linux/gpio/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/platform_device.h>

static int w152d_ao_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct gpio_regmap_config config = {
		.parent = dev,
		.ngpio = 12,
		.reg_dat_base = GPIO_REGMAP_ADDR(0),
		.reg_set_base = 4,
		.reg_dir_in_base = 8,
	};
	struct gpio_regmap *gpio;

	/* Use masked updates: AO_11 shares these registers with the LEDs. */
	config.regmap = syscon_node_to_regmap(dev->of_node);
	if (IS_ERR(config.regmap))
		return PTR_ERR(config.regmap);

	gpio = devm_gpio_regmap_register(dev, &config);
	return PTR_ERR_OR_ZERO(gpio);
}

static const struct of_device_id w152d_ao_gpio_of_match[] = {
	{ .compatible = "amlogic,w152d-ao-gpio" },
	{ }
};

static struct platform_driver w152d_ao_gpio_driver = {
	.probe = w152d_ao_gpio_probe,
	.driver = {
		.name = "w152d-ao-gpio",
		.of_match_table = w152d_ao_gpio_of_match,
	},
};
builtin_platform_driver(w152d_ao_gpio_driver);

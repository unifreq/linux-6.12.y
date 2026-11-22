// SPDX-License-Identifier: GPL-2.0-only
/* W152D CRG bus clock gates used in the RAM-only USB diagnostic. */
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(w152d_crg_clk_lock);

static int w152d_crg_clk_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct clk_hw_onecell_data *data;
	void __iomem *reg, *pipe;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);
	pipe = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(pipe))
		return PTR_ERR(pipe);

	data = devm_kzalloc(dev, struct_size(data, hws, 3), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->num = 3;
	data->hws[0] = devm_clk_hw_register_gate(dev, "w152d-usb3-crg",
			"w152d-usb-sys", 0, reg, 25, 0, &w152d_crg_clk_lock);
	if (IS_ERR(data->hws[0]))
		return PTR_ERR(data->hws[0]);
	data->hws[1] = devm_clk_hw_register_gate(dev, "w152d-usb2-crg",
			"w152d-usb-sys", 0, reg, 26, 0, &w152d_crg_clk_lock);
	if (IS_ERR(data->hws[1]))
		return PTR_ERR(data->hws[1]);
	data->hws[2] = devm_clk_hw_register_gate(dev, "w152d-usb3-pipe",
			"w152d-usb-ref", 0, pipe, 8, 0, &w152d_crg_clk_lock);
	if (IS_ERR(data->hws[2]))
		return PTR_ERR(data->hws[2]);

	return devm_of_clk_add_hw_provider(dev, of_clk_hw_onecell_get, data);
}

static const struct of_device_id w152d_crg_clk_of_match[] = {
	{ .compatible = "amlogic,w152d-crg-clock-probe" },
	{ }
};

static struct platform_driver w152d_crg_clk_driver = {
	.probe = w152d_crg_clk_probe,
	.driver = {
		.name = "w152d-crg-clock-probe",
		.of_match_table = w152d_crg_clk_of_match,
	},
};
builtin_platform_driver(w152d_crg_clk_driver);

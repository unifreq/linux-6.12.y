// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 BayLibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

#include <linux/module.h>
#include <linux/amlogic/arm-smccc.h>
#include "clk-regmap.h"

static int clk_regmap_check_is_satisfied(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_gate_data *gate = clk_get_regmap_gate_data(clk);
	unsigned int val;
	int cnt = 3;

	if (gate->check_offset) {
		do {
			if (!cnt) {
				pr_err("check %s failed!\n", clk_hw_get_name(hw));
				return -ETIMEDOUT;
			}
			/*
			 * FIXME: due to hardware reasons, the check will be delayed per 1ms,
			 * and the spinlock used during the enable period will be delayed
			 * for at least 1ms.
			 */
			udelay(1200);
			cnt--;
			regmap_read(clk->map, gate->check_offset, &val);
		} while (!(val & BIT(gate->check_bit)));
	}

	return 0;
}

static int clk_regmap_gate_endisable(struct clk_hw *hw, int enable)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_gate_data *gate = clk_get_regmap_gate_data(clk);
	int set = gate->flags & CLK_GATE_SET_TO_DISABLE ? 1 : 0;

	set ^= enable;

	return regmap_update_bits(clk->map, gate->offset, BIT(gate->bit_idx),
				  set ? BIT(gate->bit_idx) : 0);
}

static int clk_regmap_gate_enable(struct clk_hw *hw)
{
	int ret;

	ret = clk_regmap_gate_endisable(hw, 1);
	if (ret)
		return ret;

	ret = clk_regmap_check_is_satisfied(hw);
	if (ret)
		clk_regmap_gate_endisable(hw, 0);

	return ret;
}

static void clk_regmap_gate_disable(struct clk_hw *hw)
{
	if (bypass_clk_disable)
		return;

	clk_regmap_gate_endisable(hw, 0);
}

static int clk_regmap_gate_is_enabled(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_gate_data *gate = clk_get_regmap_gate_data(clk);
	unsigned int val;

	regmap_read(clk->map, gate->offset, &val);
	if (gate->flags & CLK_GATE_SET_TO_DISABLE)
		val ^= BIT(gate->bit_idx);

	val &= BIT(gate->bit_idx);

	return val ? 1 : 0;
}

static int clk_regmap_gate_save_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_gate_data *gate = clk_get_regmap_gate_data(clk);

	gate->saved_is_enabled = clk_regmap_gate_is_enabled(hw);

	return 0;
}

static void clk_regmap_gate_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_gate_data *gate = clk_get_regmap_gate_data(clk);

	clk_regmap_gate_endisable(hw, gate->saved_is_enabled);
}

const struct clk_ops clk_regmap_gate_ops = {
	.enable = clk_regmap_gate_enable,
	.disable = clk_regmap_gate_disable,
	.is_enabled = clk_regmap_gate_is_enabled,
	.save_context = clk_regmap_gate_save_context,
	.restore_context = clk_regmap_gate_restore_context,
};
EXPORT_SYMBOL_GPL(clk_regmap_gate_ops);

const struct clk_ops clk_regmap_gate_ro_ops = {
	.is_enabled = clk_regmap_gate_is_enabled,
};
EXPORT_SYMBOL_GPL(clk_regmap_gate_ro_ops);

static unsigned long clk_regmap_div_recalc_rate(struct clk_hw *hw,
						unsigned long prate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	unsigned int val;
	int ret;

	ret = regmap_read(clk->map, div->offset, &val);
	if (ret)
		/* Gives a hint that something is wrong */
		return 0;

	val >>= div->shift;
	val &= clk_div_mask(div->width);
	return divider_recalc_rate(hw, prate, val, div->table, div->flags,
				   div->width);
}

static long clk_regmap_div_round_rate(struct clk_hw *hw, unsigned long rate,
				      unsigned long *prate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	unsigned int val;
	int ret;

	/* if read only, just return current value */
	if (div->flags & CLK_DIVIDER_READ_ONLY) {
		ret = regmap_read(clk->map, div->offset, &val);
		if (ret)
			/* Gives a hint that something is wrong */
			return 0;

		val >>= div->shift;
		val &= clk_div_mask(div->width);

		return divider_ro_round_rate(hw, rate, prate, div->table,
					     div->width, div->flags, val);
	}

	return divider_round_rate(hw, rate, prate, div->table, div->width,
				  div->flags);
}

static int clk_regmap_div_set_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	unsigned int val;
	int ret;

	ret = divider_get_val(rate, parent_rate, div->table, div->width,
			      div->flags);
	if (ret < 0)
		return ret;

	val = (unsigned int)ret << div->shift;
	return regmap_update_bits(clk->map, div->offset,
				  clk_div_mask(div->width) << div->shift, val);
};

static int clk_regmap_secure_v2_div_set_rate(struct clk_hw *hw, unsigned long rate,
					     unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	unsigned int val;
	struct arm_smccc_res res;
	int ret;

	ret = divider_get_val(rate, parent_rate, div->table, div->width,
			      div->flags);
	if (ret < 0)
		return ret;

	val = (unsigned int)ret << div->shift;

	/* Send the divider to bl31, 0x8200009A is the general clk addr */
	arm_smccc_smc(div->smc_id, div->secid,
		      clk_div_mask(div->width) << div->shift, val, 0, 0, 0, 0, &res);

	return 0;
};

static int clk_regmap_div_save_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	unsigned int val;

	if (!(div->flags & CLK_DIVIDER_SECURE_IGNORE_RESTORE)) {
		regmap_read(clk->map, div->offset, &val);
		div->saved_divider = (val >> div->shift) & clk_div_mask(div->width);
	}

	return 0;
}

static void clk_regmap_div_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);

	regmap_update_bits(clk->map, div->offset,
			   clk_div_mask(div->width) << div->shift,
			   div->saved_divider << div->shift);
}

static void clk_regmap_div_secure_v2_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_div_data *div = clk_get_regmap_div_data(clk);
	struct arm_smccc_res res;

	if (!(div->flags & CLK_DIVIDER_SECURE_IGNORE_RESTORE)) {
		/* Send the saved divider to bl31, 0x8200009A is the general clk addr */
		arm_smccc_smc(div->smc_id, div->secid,
				clk_div_mask(div->width) << div->shift,
				div->saved_divider << div->shift, 0, 0, 0, 0, &res);
	}
}

/* Would prefer clk_regmap_div_ro_ops but clashes with qcom */
const struct clk_ops clk_regmap_secure_v2_divider_ops = {
	.recalc_rate = clk_regmap_div_recalc_rate,
	.round_rate = clk_regmap_div_round_rate,
	.set_rate = clk_regmap_secure_v2_div_set_rate,
	.save_context = clk_regmap_div_save_context,
	.restore_context = clk_regmap_div_secure_v2_restore_context,
};
EXPORT_SYMBOL_GPL(clk_regmap_secure_v2_divider_ops);

const struct clk_ops clk_regmap_divider_ops = {
	.recalc_rate = clk_regmap_div_recalc_rate,
	.round_rate = clk_regmap_div_round_rate,
	.set_rate = clk_regmap_div_set_rate,
	.save_context = clk_regmap_div_save_context,
	.restore_context = clk_regmap_div_restore_context,
};
EXPORT_SYMBOL_GPL(clk_regmap_divider_ops);

const struct clk_ops clk_regmap_divider_ro_ops = {
	.recalc_rate = clk_regmap_div_recalc_rate,
	.round_rate = clk_regmap_div_round_rate,
};
EXPORT_SYMBOL_GPL(clk_regmap_divider_ro_ops);

static u8 clk_regmap_mux_get_parent(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mux_data *mux = clk_get_regmap_mux_data(clk);
	unsigned int val;
	int ret;
	struct arm_smccc_res res;

	if (mux->smc_id) {
		arm_smccc_smc(mux->smc_id, mux->secid_rd,
			      0, 0, 0, 0, 0, 0, &res);
		val = res.a0;
	} else {
		ret = regmap_read(clk->map, mux->offset, &val);
		if (ret)
			return ret;
	}

	val >>= mux->shift;
	val &= mux->mask;
	return clk_mux_val_to_index(hw, mux->table, mux->flags, val);
}

static int clk_regmap_mux_set_parent(struct clk_hw *hw, u8 index)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mux_data *mux = clk_get_regmap_mux_data(clk);
	unsigned int val = clk_mux_index_to_val(mux->table, mux->flags, index);
	struct arm_smccc_res res;

	if (mux->smc_id)
		arm_smccc_smc(mux->smc_id, mux->secid,
			      mux->mask << mux->shift, val << mux->shift,
			      0, 0, 0, 0, &res);
	else
		regmap_update_bits(clk->map, mux->offset,
				   mux->mask << mux->shift,
				   val << mux->shift);

	return 0;
}

static int clk_regmap_mux_determine_rate(struct clk_hw *hw,
					 struct clk_rate_request *req)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mux_data *mux = clk_get_regmap_mux_data(clk);

	return clk_mux_determine_rate_flags(hw, req, mux->flags);
}

static int clk_regmap_mux_save_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mux_data *mux = clk_get_regmap_mux_data(clk);

	if (!mux->smc_id)
		mux->saved_parent = clk_regmap_mux_get_parent(hw);

	return 0;
}

static void clk_regmap_mux_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct clk_regmap_mux_data *mux = clk_get_regmap_mux_data(clk);
	u8 idx = mux->saved_parent;

	if (mux->smc_id)
		return;

	if (clk_hw_get_parent_by_index(hw, idx) && (clk_hw_get_flags(hw) & CLK_OPS_PARENT_ENABLE)) {
		clk_hw_set_parent(hw, clk_hw_get_parent_by_index(hw, !idx));
		clk_hw_set_parent(hw, clk_hw_get_parent_by_index(hw, idx));
	} else {
		clk_regmap_mux_set_parent(hw, mux->saved_parent);
	}
}

const struct clk_ops clk_regmap_mux_ops = {
	.get_parent = clk_regmap_mux_get_parent,
	.set_parent = clk_regmap_mux_set_parent,
	.determine_rate = clk_regmap_mux_determine_rate,
	.save_context = clk_regmap_mux_save_context,
	.restore_context = clk_regmap_mux_restore_context,
};
EXPORT_SYMBOL_GPL(clk_regmap_mux_ops);

const struct clk_ops clk_regmap_mux_ro_ops = {
	.get_parent = clk_regmap_mux_get_parent,
};
EXPORT_SYMBOL_GPL(clk_regmap_mux_ro_ops);

static struct regmap_config clkc_regmap_config = {
	.reg_bits       = 32,
	.val_bits       = 32,
	.reg_stride     = 4,
};

struct regmap *meson_clk_regmap_resource(struct platform_device *pdev, struct device *dev,
					unsigned int index)
{
	void __iomem *base;
	struct device_node *node = dev->of_node;

	base = devm_platform_ioremap_resource(pdev, index);
	if (IS_ERR(base))
		return ERR_CAST(base);

	clkc_regmap_config.name = devm_kasprintf(dev, GFP_KERNEL,
						 "%s-%d", node->name, index);

	return devm_regmap_init_mmio(dev, base, &clkc_regmap_config);
}
EXPORT_SYMBOL_GPL(meson_clk_regmap_resource);

MODULE_DESCRIPTION("Amlogic regmap backed clock driver");
MODULE_AUTHOR("Jerome Brunet <jbrunet@baylibre.com>");
MODULE_LICENSE("GPL v2");

/* vendor-tree global (extern in clk-regmap.h) */
int bypass_clk_disable = 1;

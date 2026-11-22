// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Endless Mobile, Inc.
 * Author: Carlo Caione <carlo@endlessm.com>
 *
 * Copyright (c) 2018 Baylibre, SAS.
 * Author: Jerome Brunet <jbrunet@baylibre.com>
 */

/*
 * In the most basic form, a Meson PLL is composed as follows:
 *
 *                     PLL
 *        +--------------------------------+
 *        |                                |
 *        |             +--+               |
 *  in >>-----[ /N ]--->|  |      +-----+  |
 *        |             |  |------| DCO |---->> out
 *        |  +--------->|  |      +--v--+  |
 *        |  |          +--+         |     |
 *        |  |                       |     |
 *        |  +--[ *(M + (F/Fmax) ]<--+     |
 *        |                                |
 *        +--------------------------------+
 *
 * out = in * (m + frac / frac_max) / n
 */

#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/math64.h>
#include <linux/module.h>
#include "clk-regmap.h"
#include "clk-pll.h"
#include <linux/amlogic/arm-smccc.h>

#define PLL_LOCK_RETRY_CNT_MAX		10

#define FIXED_FRAC_WEIGHT_PRECISION	100000
#define MESON_PLL_THRESHOLD_RATE	1500000000
/*
 * SoC list with threshold rate :
 *	TXHD2
 */

static inline struct meson_clk_pll_data *
meson_clk_pll_data(struct clk_regmap *clk)
{
	return (struct meson_clk_pll_data *)clk->data;
}

static int __pll_round_closest_mult(struct meson_clk_pll_data *pll)
{
	if ((pll->flags & CLK_MESON_PLL_ROUND_CLOSEST) &&
	    !MESON_PARM_APPLICABLE(&pll->frac))
		return 1;

	return 0;
}

static unsigned long __pll_params_to_rate(unsigned long parent_rate,
					  unsigned int m, unsigned int n,
					  unsigned int frac, unsigned int od,
					  struct meson_clk_pll_data *pll)
{
	u64 rate, frac_rate;
	u32 frac_precision;

	if (pll->flags & CLK_MESON_PLL_POWER_OF_TWO)
		n = (u32)BIT(n);

	if (unlikely(!n))
		return 0;

	if (pll->flags & CLK_MESON_PLL_FIXED_EN0P5)
		parent_rate = parent_rate >> 1;

	rate = DIV_ROUND_UP_ULL((u64)parent_rate * m, n);
	if (frac && MESON_PARM_APPLICABLE(&pll->frac)) {
		if (pll->flags & CLK_MESON_PLL_FIXED_FRAC_WEIGHT_PRECISION)
			frac_precision = FIXED_FRAC_WEIGHT_PRECISION;
		else
			frac_precision = pll->frac_max ?: (u32)BIT(pll->frac.width);

		frac_rate = div_u64((u64)parent_rate * frac, n);
		rate += DIV_ROUND_UP_ULL(frac_rate, frac_precision);
	}

	if (od && MESON_PARM_APPLICABLE(&pll->od))
		rate = rate >> od;

	return rate;
}

static unsigned long meson_clk_pll_recalc_rate(struct clk_hw *hw,
					       unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	unsigned int m, n, frac, od;

	n = meson_parm_read(clk->map, &pll->n);
	m = meson_parm_read(clk->map, &pll->m);

	frac = MESON_PARM_APPLICABLE(&pll->frac) ?
		meson_parm_read(clk->map, &pll->frac) :
		0;

	od = MESON_PARM_APPLICABLE(&pll->od) ?
		meson_parm_read(clk->map, &pll->od) :
		0;
	return __pll_params_to_rate(parent_rate, m, n, frac, od, pll);
}

static unsigned int __pll_params_with_frac(unsigned long rate,
					   unsigned long parent_rate,
					   unsigned int m,
					   unsigned int n,
					   unsigned int od,
					   struct meson_clk_pll_data *pll)
{
	unsigned int frac_max;
	u64 val, vco_rate = rate;

	if (MESON_PARM_APPLICABLE(&pll->od))
		vco_rate = vco_rate << od;

	if (pll->flags & CLK_MESON_PLL_FIXED_EN0P5)
		parent_rate = parent_rate >> 1;

	if (pll->flags & CLK_MESON_PLL_POWER_OF_TWO) {
		val = vco_rate << n;
		if (vco_rate < (((u64)parent_rate >> n) * m))
			return 0;
	} else {
		val = vco_rate * n;
		/* Bail out if we are already over the requested rate */
		if (!n || vco_rate < (div_u64((u64)parent_rate * m, n)))
			return 0;
	}

	if (pll->flags & CLK_MESON_PLL_FIXED_FRAC_WEIGHT_PRECISION)
		frac_max = FIXED_FRAC_WEIGHT_PRECISION;
	else
		frac_max = pll->frac_max ?: (1 << pll->frac.width);

	if (pll->flags & CLK_MESON_PLL_ROUND_CLOSEST)
		val = DIV_ROUND_CLOSEST_ULL(val * frac_max, parent_rate);
	else
		val = div_u64(val * frac_max, parent_rate);

	val -= (u64)m * frac_max;

	return min((unsigned int)val, (frac_max - 1));
}

static bool meson_clk_pll_is_better(unsigned long rate,
				    unsigned long best,
				    unsigned long now,
				    struct meson_clk_pll_data *pll)
{
	if (__pll_round_closest_mult(pll)) {
		/* Round Closest */
		if (abs(now - rate) < abs(best - rate))
			return true;
	} else {
		/* Round down */
		if (now <= rate && best < now)
			return true;
	}

	return false;
}

static int meson_clk_get_pll_table_index(unsigned int index,
					 unsigned int *m,
					 unsigned int *n,
					 unsigned int *od,
					 struct meson_clk_pll_data *pll)
{
	/* In some SoCs, n = 0, so check m here */
	if (!pll->table[index].m)
		return -EINVAL;

	*m = pll->table[index].m;
	*n = pll->table[index].n;
	if (MESON_PARM_APPLICABLE(&pll->od))
		*od = pll->table[index].od;

	return 0;
}

static unsigned int meson_clk_get_pll_range_m(unsigned long long rate,
					      unsigned long parent_rate,
					      unsigned int n,
					      struct meson_clk_pll_data *pll)
{
	u64 val;

	if (pll->flags & CLK_MESON_PLL_POWER_OF_TWO)
		val = rate << n;
	else
		val = rate * n;
	if (__pll_round_closest_mult(pll))
		return DIV_ROUND_CLOSEST_ULL(val, parent_rate);

	return div_u64(val,  parent_rate);
}

static int meson_clk_get_pll_range_index(unsigned long rate,
					 unsigned long parent_rate,
					 unsigned int index,
					 unsigned int *m,
					 unsigned int *n,
					 unsigned int *od,
					 struct meson_clk_pll_data *pll)
{
	unsigned int idx, od_max, t_m;
	u64 vco_rate;
	unsigned long now, best = 0;

	if (((pll->flags & CLK_MESON_PLL_FIXED_N) && index > 0) ||
	    (!(pll->flags & CLK_MESON_PLL_FIXED_N) &&
	     (index >= (1 << pll->n.width))))
		return -EINVAL;

	if (pll->flags & CLK_MESON_PLL_FIXED_N) {
		if (pll->flags & CLK_MESON_PLL_POWER_OF_TWO)
			*n = 0;
		else
			*n = 1;
	} else {
		if (pll->flags & CLK_MESON_PLL_POWER_OF_TWO)
			*n = index;
		else
			*n = index + 1;
	}

	if (pll->flags & CLK_MESON_PLL_FIXED_EN0P5)
		parent_rate = parent_rate >> 1;

	if (!MESON_PARM_APPLICABLE(&pll->od)) {
		if (index == 0) {
			/* Get the boundaries out the way */
			if (rate <= pll->range->min * parent_rate) {
				*m = pll->range->min;
				return -ENODATA;
			} else if (rate >= pll->range->max * parent_rate) {
				*m = pll->range->max;
				return -ENODATA;
			}
		}

		*m = meson_clk_get_pll_range_m(rate, parent_rate, *n, pll);
	} else {
		/* limit od max value for hw limit */
		if (unlikely(!!pll->od_max))
			od_max = pll->od_max;
		else
			od_max = BIT(pll->od.width) - 1;

		for (idx = 0; idx <= od_max; idx++) {
			/*
			 * All post-PLL od divider have the CLK_POWER_OF_TWO
			 * feature.
			 */
			vco_rate = (u64)rate << idx;
			t_m = meson_clk_get_pll_range_m(vco_rate, parent_rate, *n, pll);
			if (t_m > pll->range->max)
				break;
			if (t_m < pll->range->min)
				continue;

			now = __pll_params_to_rate(parent_rate, t_m, *n, 0, idx, pll);
			if (meson_clk_pll_is_better(rate, best, now, pll)) {
				best = now;
				*od = idx;
				*m = t_m;

				if (now == rate)
					break;
			}
		}
	}

	/* the pre-divider gives a multiplier too big - stop */
	if (*m > pll->range->max)
		return -EINVAL;

	return 0;
}

static int meson_clk_get_pll_get_index(unsigned long rate,
				       unsigned long parent_rate,
				       unsigned int index,
				       unsigned int *m,
				       unsigned int *n,
				       unsigned int *od,
				       struct meson_clk_pll_data *pll)
{
	if (pll->range)
		return meson_clk_get_pll_range_index(rate, parent_rate,
						     index, m, n, od, pll);
	else if (pll->table)
		return meson_clk_get_pll_table_index(index, m, n, od, pll);

	return -EINVAL;
}

static int meson_clk_get_pll_settings(unsigned long rate,
				      unsigned long parent_rate,
				      unsigned int *best_m,
				      unsigned int *best_n,
				      unsigned int *best_od,
				      struct meson_clk_pll_data *pll)
{
	unsigned long best = 0, now = 0;
	unsigned int i, m, n, od = 0;
	int ret;

	for (i = 0, ret = 0; !ret; i++) {
		ret = meson_clk_get_pll_get_index(rate, parent_rate,
						  i, &m, &n, &od, pll);
		if (ret == -EINVAL)
			break;

		now = __pll_params_to_rate(parent_rate, m, n, 0, od, pll);
		if (meson_clk_pll_is_better(rate, best, now, pll)) {
			best = now;
			*best_m = m;
			*best_n = n;
			if (MESON_PARM_APPLICABLE(&pll->od))
				*best_od = od;

			if (now == rate)
				break;
		}
	}

	return best ? 0 : -EINVAL;
}

static int meson_clk_pll_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	unsigned int m, n, frac = 0, od;
	int ret;

	if (pll->flags & CLK_MESON_PLL_READ_ONLY) {
		req->rate = clk_hw_get_rate(hw);
		return 0;
	}

	ret = meson_clk_get_pll_settings(req->rate, req->best_parent_rate,
					 &m, &n, &od, pll);
	if (ret)
		return ret;

	if (MESON_PARM_APPLICABLE(&pll->frac))
		frac = __pll_params_with_frac(req->rate, req->best_parent_rate,
					      m, n, od, pll);

	req->rate = __pll_params_to_rate(req->best_parent_rate, m, n, frac, od,
					 pll);

	return 0;
}

static int meson_clk_pll_wait_lock(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	int delay = 1000;

	do {
		/* Is the clock locked now ? */
		if (meson_parm_read(clk->map, &pll->l))
			return 0;
		udelay(1);
	} while (delay--);

	return -ETIMEDOUT;
}

static int meson_clk_pll_is_enabled(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	/* Enable and lock bit equal 1, it locks */
	if (meson_parm_read(clk->map, &pll->en) &&
	    meson_parm_read(clk->map, &pll->l))
		return 1;

	return 0;
}

static int meson_clk_pll_init(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	/*
	 * Keep the clock running, which was already initialized and enabled
	 * from the bootloader stage, to avoid any glitches.
	 */
	if ((pll->flags & CLK_MESON_PLL_NOINIT_ENABLED) &&
	    meson_clk_pll_is_enabled(hw))
		return 0;

	if (pll->init_count) {
		if (pll->flags & CLK_MESON_PLL_RSTN) {
			meson_parm_write(clk->map, &pll->rst, 0);
			regmap_multi_reg_write(clk->map, pll->init_regs,
				       pll->init_count);
			meson_parm_write(clk->map, &pll->rst, 1);
		} else {
			meson_parm_write(clk->map, &pll->rst, 1);
			regmap_multi_reg_write(clk->map, pll->init_regs,
				       pll->init_count);
			meson_parm_write(clk->map, &pll->rst, 0);
		}
	}

	return 0;
}

static int meson_clk_pcie_pll_enable(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	int retry = 10, ret = 10;

	do {
		meson_clk_pll_init(hw);
		if (!meson_clk_pll_wait_lock(hw)) {
			/* what means here ? */
			if (!MESON_PARM_APPLICABLE(&pll->pcie_hcsl))
				return 0;
			break;
		}
		pr_info("%s:%d retry = %d\n", __func__, __LINE__, retry);
	} while (retry--);

	if (retry <= 0)
		return -EIO;

	if (!MESON_PARM_APPLICABLE(&pll->pcie_hcsl))
		return 0;

	/*pcie pll clk share use for usb phy, so add this operation from ASIC*/
	do {
		if (meson_parm_read(clk->map, &pll->pcie_hcsl)) {
			meson_parm_write(clk->map, &pll->pcie_exen, 0);
			return 0;
		}
		udelay(1);
	} while (ret--);

	if (ret <= 0)
		pr_info("%s:%d pcie reg1 clear bit29 failed\n", __func__, __LINE__);

	return -EIO;
}

static void meson_clk_pll_disable(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	if (bypass_clk_disable)
		return;

	/* Put the pll is in reset */
	if (MESON_PARM_APPLICABLE(&pll->rst)) {
		if (pll->flags & CLK_MESON_PLL_RSTN)
			meson_parm_write(clk->map, &pll->rst, 0);
		else
			meson_parm_write(clk->map, &pll->rst, 1);
	}

	/* Disable the pll */
	meson_parm_write(clk->map, &pll->en, 0);
}

static int meson_clk_pll_enable(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	int retry = 0;

	/* do nothing if the PLL is already enabled */
	if (clk_hw_is_enabled(hw))
		return 0;

	do {
		/* Make sure the pll is disabled */
		meson_clk_pll_disable(hw);

		/* Make sure bit is 0 before enable pll */
		if (MESON_PARM_APPLICABLE(&pll->current_en))
			meson_parm_write(clk->map, &pll->current_en, 0);

		/* Make sure the pll is in lock reset */
		if (MESON_PARM_APPLICABLE(&pll->l_rst)) {
			if (pll->flags & CLK_MESON_PLL_L_RSTN)
				meson_parm_write(clk->map, &pll->l_rst, 0);
			else
				meson_parm_write(clk->map, &pll->l_rst, 1);
		}

		/* Enable the pll */
		meson_parm_write(clk->map, &pll->en, 1);

		/* Make sure the pll force lock is clear */
		if (MESON_PARM_APPLICABLE(&pll->fl))
			meson_parm_write(clk->map, &pll->fl, 0);

		/*
		 * Compared with the previous SoCs, self-adaption current module
		 * is newly added for A1, keep the new power-on sequence to enable the
		 * PLL. The sequence is:
		 * 1. enable the pll, delay for 10us
		 * 2. enable the pll self-adaption current module, delay for 40us
		 * 3. enable the lock detect module
		 */
		if (MESON_PARM_APPLICABLE(&pll->current_en)) {
			udelay(10);
			meson_parm_write(clk->map, &pll->current_en, 1);
			udelay(40);
		} else {
			/*
			 * Delay 50us to wait for PLL to stabilize, otherwise PLL may
			 * have a lock failure (especially in low temperature scenarios).
			 */
			udelay(50);
		}

		if (MESON_PARM_APPLICABLE(&pll->l_detect)) {
			meson_parm_write(clk->map, &pll->l_detect, 1);
			meson_parm_write(clk->map, &pll->l_detect, 0);
		}

		/* Take the pll out reset */
		if (MESON_PARM_APPLICABLE(&pll->rst)) {
			if (pll->flags & CLK_MESON_PLL_RSTN)
				meson_parm_write(clk->map, &pll->rst, 1);
			else
				meson_parm_write(clk->map, &pll->rst, 0);
		}

		/* Take the pll out lock reset */
		if (MESON_PARM_APPLICABLE(&pll->l_rst)) {
			udelay(20);
			if (pll->flags & CLK_MESON_PLL_L_RSTN)
				meson_parm_write(clk->map, &pll->l_rst, 1);
			else
				meson_parm_write(clk->map, &pll->l_rst, 0);
		}

		if (!meson_clk_pll_wait_lock(hw)) {
			/* Make sure the pll force lock is set */
			if (MESON_PARM_APPLICABLE(&pll->fl))
				meson_parm_write(clk->map, &pll->fl, 1);

			return 0;
		}
	} while (++retry < PLL_LOCK_RETRY_CNT_MAX);

	pr_warn("%s: PLL lock failed!!!\n", clk_hw_get_name(hw));

	return -EIO;
}

static int meson_clk_pll_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	unsigned int enabled, m, n, frac, od;
	int ret;

	if (parent_rate == 0 || rate == 0)
		return -EINVAL;

	ret = meson_clk_get_pll_settings(rate, parent_rate, &m, &n, &od, pll);
	if (ret)
		return ret;

	enabled = meson_clk_pll_is_enabled(hw);

	/* Don't disable pll if it's just changing frac */
	if ((meson_parm_read(clk->map, &pll->m) != m ||
	     meson_parm_read(clk->map, &pll->n) != n) && enabled)
		meson_clk_pll_disable(hw);

	meson_parm_write(clk->map, &pll->n, n);
	meson_parm_write(clk->map, &pll->m, m);

	if (MESON_PARM_APPLICABLE(&pll->frac)) {
		frac = __pll_params_with_frac(rate, parent_rate, m, n, od, pll);
		meson_parm_write(clk->map, &pll->frac, frac);
	}

	if (MESON_PARM_APPLICABLE(&pll->od))
		meson_parm_write(clk->map, &pll->od, od);

	/* If the pll is stopped, bail out now */
	if (!enabled)
		return 0;

	return meson_clk_pll_enable(hw);
}

static int meson_clk_pll_save_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	int i;

	pll->saved_rate = meson_clk_pll_recalc_rate(hw,
			    clk_hw_get_rate(clk_hw_get_parent(hw)));
	pll->saved_is_enabled = meson_clk_pll_is_enabled(hw);

	for (i = 0; i < pll->save_reg_count; i++)
		regmap_read(clk->map, pll->save_regs[i].reg,
			    &pll->save_regs[i].def);

	return 0;
}

static void meson_clk_pll_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	if (!meson_clk_pll_is_enabled(hw) && pll->init_count) {
		meson_parm_write(clk->map, &pll->rst, 1);
		regmap_multi_reg_write(clk->map, pll->init_regs,
				pll->init_count);
		meson_parm_write(clk->map, &pll->rst, 0);

		regmap_multi_reg_write(clk->map, pll->save_regs,
				       pll->save_reg_count);
	}

	meson_clk_pll_set_rate(hw, pll->saved_rate,
			clk_hw_get_rate(clk_hw_get_parent(hw)));
	if (pll->saved_is_enabled)
		meson_clk_pll_enable(hw);
	else
		meson_clk_pll_disable(hw);
}

static int meson_clk_pcie_pll_save_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	pll->saved_is_enabled = meson_clk_pll_is_enabled(hw);

	return 0;
}

static void meson_clk_pcie_pll_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	if (pll->saved_is_enabled)
		meson_clk_pcie_pll_enable(hw);
	else
		meson_clk_pll_disable(hw);
}

/*
 * The Meson G12A PCIE PLL is fined tuned to deliver a very precise
 * 100MHz reference clock for the PCIe Analog PHY, and thus requires
 * a strict register sequence to enable the PLL.
 * To simplify, re-use the _init() op to enable the PLL and keep
 * the other ops except set_rate since the rate is fixed.
 */
const struct clk_ops meson_clk_pcie_pll_ops = {
	.recalc_rate	= meson_clk_pll_recalc_rate,
	.determine_rate	= meson_clk_pll_determine_rate,
	.is_enabled	= meson_clk_pll_is_enabled,
	.enable		= meson_clk_pcie_pll_enable,
	.disable	= meson_clk_pll_disable,
	.save_context	= meson_clk_pcie_pll_save_context,
	.restore_context = meson_clk_pcie_pll_restore_context,
};
EXPORT_SYMBOL_GPL(meson_clk_pcie_pll_ops);

const struct clk_ops meson_clk_pll_ops = {
	.init		= meson_clk_pll_init,
	.recalc_rate	= meson_clk_pll_recalc_rate,
	.determine_rate	= meson_clk_pll_determine_rate,
	.set_rate	= meson_clk_pll_set_rate,
	.is_enabled	= meson_clk_pll_is_enabled,
	.enable		= meson_clk_pll_enable,
	.disable	= meson_clk_pll_disable,
	.save_context	= meson_clk_pll_save_context,
	.restore_context = meson_clk_pll_restore_context,
};
EXPORT_SYMBOL_GPL(meson_clk_pll_ops);

static void meson_secure_pll_v2_disable(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	struct arm_smccc_res res;

	if (bypass_clk_disable)
		return;

	arm_smccc_smc(pll->smc_id, pll->secid_disable,
		      0, 0, 0, 0, 0, 0, &res);
}

static int meson_secure_pll_v2_set_rate(struct clk_hw *hw, unsigned long rate,
					unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	struct arm_smccc_res res;
	unsigned int m, n, od = 0;
	int ret;

	if (parent_rate == 0 || rate == 0)
		return -EINVAL;

	ret = meson_clk_get_pll_settings(rate, parent_rate, &m, &n, &od, pll);
	if (ret)
		return ret;

	if (meson_parm_read(clk->map, &pll->en))
		meson_secure_pll_v2_disable(hw);

	arm_smccc_smc(pll->smc_id, pll->secid,
		      m, n, od, 0, 0, 0, &res);

	return 0;
}

static int meson_secure_pll_v2_enable(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	unsigned int m, n, od;
	struct arm_smccc_res res;

	/*
	 * In most case,  do nothing if the PLL is already enabled
	 */
	if (clk_hw_is_enabled(hw))
		return 0;

	/* If PLL is not enabled because setting the same rate,
	 * Enable it again, CCF will return when set the same rate
	 */
	n = meson_parm_read(clk->map, &pll->n);
	m = meson_parm_read(clk->map, &pll->m);
	/* od is required in arm64 and arm */
	od = MESON_PARM_APPLICABLE(&pll->od) ?
	     meson_parm_read(clk->map, &pll->od) : 0;

	arm_smccc_smc(pll->smc_id, pll->secid,
		      m, n, od, 0, 0, 0, &res);

	return 0;
}

static void meson_secure_pll_v2_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	if (!meson_clk_pll_is_enabled(hw)) {
		meson_secure_pll_v2_set_rate(hw, pll->saved_rate,
					     clk_hw_get_rate(clk_hw_get_parent(hw)));
		if (pll->saved_is_enabled)
			meson_secure_pll_v2_enable(hw);
		else
			meson_secure_pll_v2_disable(hw);
	}
}

const struct clk_ops meson_secure_pll_v2_ops = {
	.recalc_rate	= meson_clk_pll_recalc_rate,
	.determine_rate	= meson_clk_pll_determine_rate,
	.set_rate	= meson_secure_pll_v2_set_rate,
	.is_enabled	= meson_clk_pll_is_enabled,
	.enable		= meson_secure_pll_v2_enable,
	.disable	= meson_secure_pll_v2_disable,
	.save_context	= meson_clk_pll_save_context,
	.restore_context = meson_secure_pll_v2_restore_context,
};
EXPORT_SYMBOL_GPL(meson_secure_pll_v2_ops);

static int meson_clk_pll_v3_set_rate(struct clk_hw *hw, unsigned long rate,
				  unsigned long parent_rate)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);
	struct parm *pm = &pll->m;
	struct parm *pn = &pll->n;
	struct parm *pth = &pll->th;
	struct parm *pfrac = &pll->frac;
	struct parm *pod = &pll->od;
	struct parm *pfl = &pll->fl;
	unsigned int m, n, frac, od;
	unsigned int val;
	const struct reg_sequence *init_regs = pll->init_regs;
	int i, ret = 0, retry = 10;

	if (parent_rate == 0 || rate == 0)
		return -EINVAL;

	/* calculate M, N, OD*/
	ret = meson_clk_get_pll_settings(rate, parent_rate, &m, &n, &od, pll);
	if (ret)
		return ret;
	/* calculate frac if need*/
	if (MESON_PARM_APPLICABLE(&pll->frac))
		frac = __pll_params_with_frac(rate, parent_rate, m, n, od, pll);

	if (meson_parm_read(clk->map, &pll->m) == m &&
	    meson_parm_read(clk->map, &pll->n) == n &&
	    meson_parm_read(clk->map, &pll->en)) {
		if (MESON_PARM_APPLICABLE(&pll->od))
			meson_parm_write(clk->map, pod, od);

		if (MESON_PARM_APPLICABLE(&pll->frac))
			meson_parm_write(clk->map, pfrac, frac);

		if (!meson_clk_pll_wait_lock(hw))
			return 0;
	}

	if (meson_parm_read(clk->map, &pll->en))
		meson_clk_pll_disable(hw);

	do {
		for (i = 0; i < pll->init_count; i++) {
			if (pn->reg_off == init_regs[i].reg) {
				/* Clear M N bits and Update M N value */
				val = init_regs[i].def;
				if (MESON_PARM_APPLICABLE(pth)) {
					val &= CLRPMASK(pth->width, pth->shift);
					if (__pll_params_to_rate(parent_rate, m,
								 n, frac, od,
								 pll) >=
					    MESON_PLL_THRESHOLD_RATE)
						val |= 1 << pth->shift;
				}
				val &= CLRPMASK(pn->width, pn->shift);
				val &= CLRPMASK(pm->width, pm->shift);
				val |= n << pn->shift;
				val |= m << pm->shift;
				if (MESON_PARM_APPLICABLE(pod)) {
					val &= CLRPMASK(pod->width, pod->shift);
					val |= od << pod->shift;
				}
				regmap_write(clk->map, pn->reg_off, val);
			} else if (pfrac->reg_off == init_regs[i].reg &&
				(MESON_PARM_APPLICABLE(pfrac))) {
				/* Clear Frac bits and Update Frac value */
				val = init_regs[i].def;
				val &= CLRPMASK(pfrac->width, pfrac->shift);
				val |= frac << pfrac->shift;
				regmap_write(clk->map, pfrac->reg_off, val);
			} else if (MESON_PARM_APPLICABLE(pfl) &&
				   pfl->reg_off == init_regs[i].reg) {
				val = init_regs[i].def;
				val &= CLRPMASK(pfl->width, pfl->shift);
				regmap_write(clk->map, pfl->reg_off, val);
			} else {
				val = init_regs[i].def;
				regmap_write(clk->map, init_regs[i].reg, val);
			}
			if (init_regs[i].delay_us)
				udelay(init_regs[i].delay_us);
		}

		if (!meson_clk_pll_wait_lock(hw)) {
			/*
			 * In special scenarios (such as ESD interference), the pll clock
			 * source may fluctuate due to interference, and the fluctuations
			 * will recover in a short time. enable the fl bit to prevent the
			 * pll from ignoring interference and force lock
			 */
			if (MESON_PARM_APPLICABLE(pfl))
				regmap_update_bits(clk->map, pfl->reg_off, BIT(pfl->shift),
						   BIT(pfl->shift));

			break;
		}
		retry--;
	} while (retry > 0);

	if (retry == 0) {
		pr_err("%s pll locked failed\n", clk_hw_get_name(hw));
		ret = -EIO;
	}

	return ret;
}

static int meson_clk_pll_v3_enable(struct clk_hw *hw)
{
	unsigned long rate, parent_rate;

	/* do nothing if the PLL is already enabled */
	if (clk_hw_is_enabled(hw))
		return 0;

	/* Deal clk_set_rate return when set the same rate */
	parent_rate = clk_hw_get_rate(clk_hw_get_parent(hw));
	rate = meson_clk_pll_recalc_rate(hw, parent_rate);
	meson_clk_pll_v3_set_rate(hw, rate, parent_rate);

	if (meson_clk_pll_wait_lock(hw))
		return -EIO;

	return 0;
}

static void meson_clk_pll_v3_restore_context(struct clk_hw *hw)
{
	struct clk_regmap *clk = to_clk_regmap(hw);
	struct meson_clk_pll_data *pll = meson_clk_pll_data(clk);

	meson_clk_pll_v3_set_rate(hw, pll->saved_rate,
				  clk_hw_get_rate(clk_hw_get_parent(hw)));
	if (pll->saved_is_enabled)
		meson_clk_pll_v3_enable(hw);
	else
		meson_clk_pll_disable(hw);
}

const struct clk_ops meson_clk_pll_v3_ops = {
	/* walk the init regs each time when set a new rate,
	 * init callback is not useful for v3 ops
	 */
	.recalc_rate	= meson_clk_pll_recalc_rate,
	.determine_rate	= meson_clk_pll_determine_rate,
	.set_rate	= meson_clk_pll_v3_set_rate,
	.is_enabled	= meson_clk_pll_is_enabled,
	.enable		= meson_clk_pll_v3_enable,
	.disable	= meson_clk_pll_disable,
	.save_context	= meson_clk_pll_save_context,
	.restore_context = meson_clk_pll_v3_restore_context,
};
EXPORT_SYMBOL_GPL(meson_clk_pll_v3_ops);
const struct clk_ops meson_clk_pll_ro_ops = {
	.recalc_rate	= meson_clk_pll_recalc_rate,
	.is_enabled	= meson_clk_pll_is_enabled,
};
EXPORT_SYMBOL_GPL(meson_clk_pll_ro_ops);

MODULE_DESCRIPTION("Amlogic PLL driver");
MODULE_AUTHOR("Carlo Caione <carlo@endlessm.com>");
MODULE_AUTHOR("Jerome Brunet <jbrunet@baylibre.com>");
MODULE_LICENSE("GPL v2");

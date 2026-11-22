/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */
#ifndef __LINUX_AMLOGIC_ARM_SMCCC_H
#define __LINUX_AMLOGIC_ARM_SMCCC_H

#include <linux/arm-smccc.h>

#if IS_REACHABLE(CONFIG_AMLOGIC_DEBUG)
#undef arm_smccc_smc
#undef arm_smccc_smc_quirk

void __arm_smccc_smc_glue(unsigned long a0, unsigned long a1,
			unsigned long a2, unsigned long a3, unsigned long a4,
			unsigned long a5, unsigned long a6, unsigned long a7,
			struct arm_smccc_res *res, struct arm_smccc_quirk *quirk);

#define arm_smccc_smc(...) __arm_smccc_smc_glue(__VA_ARGS__, NULL)

#define arm_smccc_smc_quirk(...) __arm_smccc_smc_glue(__VA_ARGS__)
#endif

#endif /*__LINUX_AMLOGIC_ARM_SMCCC_H*/

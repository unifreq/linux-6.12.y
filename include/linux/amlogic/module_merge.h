/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#ifndef _MODULE_MERGE_H_
#define _MODULE_MERGE_H_

#include <linux/amlogic/gki_module.h>

#define call_sub_init(func) \
{ \
	int ret = 0; \
	ret = func(); \
	if (ret < 0) \
		pr_err("call %s() ret=%d\n", #func, ret); \
	else \
		pr_debug("call %s() success\n", #func); \
}

#endif /* _MODULE_MERGE_H_ */

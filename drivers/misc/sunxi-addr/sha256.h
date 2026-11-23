/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __SUNXI_ADDR_SHA256_H
#define __SUNXI_ADDR_SHA256_H

#include <linux/types.h>

int hmac_sha256(const uint8_t *plaintext, ssize_t psize, uint8_t *output);

#endif /* __SUNXI_ADDR_SHA256_H */

/*
 *  Parser for NAND and EMMC Flash have different partition path.
 *
 *  WCN partition Parser module header.
 *
 *  Copyright (C) 2017 Spreadtrum Company
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 */

#ifndef _WCN_PARN_PARSER
#define _WCN_PARN_PARSER

/*
 * The parser writes into a plain char * and cannot query its size, so the
 * buffer size is part of the interface: callers must provide at least this
 * many bytes. The value covers the longest path the parser can produce from
 * a maximum-length fstab line.
 */
#define WCN_FIRMWARE_PATH_MAX 256

int parse_firmware_path(char *firmware_path);

#endif

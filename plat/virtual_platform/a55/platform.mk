#
# Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Common Build Options
#PROGRAMMABLE_RESET_ADDRESS := 1
#COLD_BOOT_SINGLE_CPU := 1
USE_COHERENT_MEM = 0
HW_ASSISTED_COHERENCY = 1
#CTX_INCLUDE_AARCH32_REGS = 0
#GENERATE_COT = 0
#TRUSTED_BOARD_BOOT = 0
#CRASH_REPORTING = 0

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=      ${XLAT_TABLES_LIB_SRCS}

PLAT_INCLUDES		:=	-Iplat/virtual_platform/a55/include	\
						-Iinclude/drivers/ti/uart			\
						-Iinclude/drivers/io	\
						-Iinclude

BL1_SOURCES		+=	plat/virtual_platform/a55/a55_helpers.S	\
					lib/cpus/aarch64/cortex_a55.S	\
					plat/virtual_platform/a55/a55_io_storage.c	\
					plat/virtual_platform/a55/a55_bl1_setup.c	\
					drivers/io/io_semihosting.c	\
					drivers/io/io_storage.c					\
					drivers/io/io_fip.c					\
					drivers/io/io_memmap.c				\
					drivers/ti/uart/${ARCH}/16550_console.S


BL2_SOURCES		+=	plat/virtual_platform/a55/a55_helpers.S	\
					drivers/ti/uart/${ARCH}/16550_console.S			\
					plat/virtual_platform/a55/a55_bl2_setup.c		\
					plat/virtual_platform/a55/a55_io_storage.c	\
					drivers/io/io_storage.c					\
					drivers/io/io_fip.c					\
					drivers/io/io_memmap.c				\
					common/desc_image_load.c					\
					plat/virtual_platform/a55/a55_bl2_mem_params_desc.c


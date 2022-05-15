#
# Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Set to armv7, default is armv8, some feature not exist on a15, will exception
ARM_ARCH_MAJOR		:= 7

# Define for gcc set -mcpu, so it support smc #0, armv7 default is not support
ARM_CORTEX_A15=yes

# For use xlat table v2, only can used in armv7 which support LPA
$(eval $(call add_define,ARMV7_SUPPORTS_LARGE_PAGE_ADDRESSING))

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=      ${XLAT_TABLES_LIB_SRCS}

PLAT_INCLUDES		:=	-Iplat/virtual_platform/a15/include	\
						-Iinclude/drivers/io	\
						-Iinclude

BL1_SOURCES		+=	plat/virtual_platform/a15/a15_helpers.S	\
					lib/cpus/aarch32/cortex_a15.S	\
					plat/virtual_platform/a15/a15_io_storage.c	\
					plat/virtual_platform/a15/a15_bl1_setup.c	\
					drivers/io/io_semihosting.c	\
					drivers/io/io_storage.c					\
					drivers/io/io_fip.c					\
					drivers/io/io_memmap.c				\
					drivers/arm/pl011/aarch32/pl011_console.S

BL2_SOURCES		+=	plat/virtual_platform/a15/a15_helpers.S	\
					drivers/arm/pl011/aarch32/pl011_console.S			\
					plat/virtual_platform/a15/a15_bl2_setup.c		\
					plat/virtual_platform/a15/a15_io_storage.c	\
					drivers/io/io_storage.c					\
					drivers/io/io_fip.c					\
					drivers/io/io_memmap.c				\
					common/desc_image_load.c					\
					plat/virtual_platform/a15/a15_bl2_mem_params_desc.c

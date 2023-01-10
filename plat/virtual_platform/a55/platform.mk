#
# Copyright (c) 2013-2022, Arm Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# Common Build Options
PROGRAMMABLE_RESET_ADDRESS := 1
#COLD_BOOT_SINGLE_CPU := 1
USE_COHERENT_MEM = 0
HW_ASSISTED_COHERENCY = 1
#CTX_INCLUDE_AARCH32_REGS = 0
#GENERATE_COT = 0
#TRUSTED_BOARD_BOOT = 0
#CRASH_REPORTING = 0
ERRATA_A55_1530923 := 1

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=      ${XLAT_TABLES_LIB_SRCS}

PLAT_INCLUDES		:=	-Iplat/virtual_platform/a55/include	\
						-Iinclude/drivers/arm				\
						-Iinclude/drivers/ti/uart			\
						-Iinclude/drivers/io	\
						-Iinclude

BL1_SOURCES		+=	plat/virtual_platform/a55/a55_helpers.S	\
					lib/cpus/aarch64/cortex_a55.S	\
					plat/virtual_platform/a55/a55_io_storage.c	\
					plat/virtual_platform/a55/a55_bl1_setup.c	\
					plat/virtual_platform/a55/a55_common.c	\
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
					plat/virtual_platform/a55/a55_common.c		\
					plat/virtual_platform/a55/a55_bl2_mem_params_desc.c

# Include GICv3 driver files
include drivers/arm/gic/v3/gicv3.mk

A55_GIC_SOURCES		+=	\
				${GICV3_SOURCES}			\
				plat/common/plat_gicv3.c


BL31_SOURCES		+=	\
				plat/virtual_platform/a55/a55_helpers.S	\
				plat/common/plat_psci_common.c			\
				lib/cpus/aarch64/cortex_a55.S		\
				drivers/ti/uart/${ARCH}/16550_console.S		\
				plat/virtual_platform/a55/a55_bl31_setup.c		\
				plat/virtual_platform/a55/a55_topology.c		\
				plat/virtual_platform/a55/a55_common.c		\
				plat/virtual_platform/a55/a55_pm.c		\
				${A55_GIC_SOURCES}


# SPD (Secure-EL1 Payload Dispatcher) details is described in services/spd/* modules
# source code , please refer to it if you want to know more
ifeq (${SPD},opteed)
BL2_SOURCES	+=	lib/optee/optee_utils.c
endif

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif

ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif
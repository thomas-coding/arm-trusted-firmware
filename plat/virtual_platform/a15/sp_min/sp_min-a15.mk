#
# SP MIN compilation top level settings for Alius SoC.
#
# Maintainer: Liu Wei <Wei.Liu@verisilicon.com>
#
# Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
#

# GICV3 Support
include drivers/arm/gic/v3/gicv3.mk
A15_GIC_SOURCES := ${GICV3_SOURCES}					\
                    plat/arm/common/arm_gicv3.c				\

BL32_SOURCES += plat/common/aarch32/platform_mp_stack.S			\
					      lib/cpus/aarch32/cortex_a15.S	\
                plat/virtual_platform/a15/a15_helpers.S	\
					      drivers/arm/pl011/aarch32/pl011_console.S			\
                plat/common/plat_psci_common.c				\
                plat/virtual_platform/a15/sp_min/a15_sp_min_setup.c   \
                plat/virtual_platform/a15/sp_min/a15_topology.c   \
                plat/virtual_platform/a15/sp_min/a15_pm.c   \
                ${A15_GIC_SOURCES}					\


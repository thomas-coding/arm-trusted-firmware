// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 */

#include <common/debug.h>
#include <uart_16550.h>
#include <platform_def.h>
#include <a55_private.h>

//For mmu
#include <lib/xlat_tables/xlat_tables_v2.h>

#define MAP_UART	MAP_REGION_FLAT(PLAT_A55_UART_BASE,	\
					PLAT_A55_UART_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_FLASH	MAP_REGION_FLAT(BL_FLASH_BASE,	\
					BL_FLASH_SIZE,	\
					MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_DDR	MAP_REGION_FLAT(BL_DDR_BASE,	\
					BL_DDR_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#ifdef IMAGE_BL1
static const mmap_region_t plat_a55_mmap[] = {
	MAP_UART,
	MAP_FLASH,
	{0}
};
#endif

#ifdef IMAGE_BL2
static const mmap_region_t plat_a55_mmap[] = {
	MAP_UART,
	MAP_FLASH,
	MAP_DDR,
	{0}
};
#endif

#ifdef IMAGE_BL31
static const mmap_region_t plat_a55_mmap[] = {
	MAP_UART,
	MAP_FLASH,
	MAP_DDR,
	{0}
};
#endif

/*******************************************************************************
 * Macro generating the code for the function setting up the pagetables as per
 * the platform memory map & initialize the mmu, for the given exception level
 ******************************************************************************/
#define DEFINE_CONFIGURE_MMU_EL(_el)					\
	void a55_configure_mmu_##_el(unsigned long total_base,	\
				   unsigned long total_size,		\
				   unsigned long code_start,		\
				   unsigned long code_limit)		\
	{								\
		mmap_add_region(total_base, total_base,			\
				total_size,				\
				MT_MEMORY | MT_RW | MT_SECURE);		\
		mmap_add_region(code_start, code_start,			\
				code_limit - code_start,		\
				MT_CODE | MT_SECURE);			\
		mmap_add(plat_a55_mmap);				\
		init_xlat_tables();					\
									\
		enable_mmu_##_el(0);					\
	}

/* Define EL1 and EL3 variants of the function initialising the MMU */
DEFINE_CONFIGURE_MMU_EL(el1)
DEFINE_CONFIGURE_MMU_EL(el3)

unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_A55_OSC_CLK;
}

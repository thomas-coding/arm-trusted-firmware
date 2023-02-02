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

#define MAP_GICD	MAP_REGION_FLAT(PLAT_ARM_GICD_BASE,		\
					PLAT_ARM_GICD_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_GICR	MAP_REGION_FLAT(PLAT_ARM_GICR_BASE,		\
					PLAT_ARM_GICR_SIZE,		\
					MT_DEVICE | MT_RW | MT_SECURE)

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
	MAP_GICD,
	MAP_GICR,
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

uint32_t a55_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

#include <common/debug.h>

/*
 * Show necessary register information in exception.
 */
void a55_report_exception(unsigned int exception_type)
{
	uint8_t el = get_current_el();
	uint64_t spsr = 0;
	uint64_t elr = 0;
	uint64_t esr = 0;
	uint64_t far = 0;

	ERROR("*********************************************\n");
	ERROR("Exception: type 0x%x\n", exception_type);
	ERROR("\tCurrent exception level: EL%d\n", el);
	if (IS_IN_EL1()) {
		spsr = read_spsr_el1();
		elr = read_elr_el1();
		esr = read_esr_el1();
		far = read_far_el1();
	} else if (IS_IN_EL3()) {
		spsr = read_spsr_el3();
		elr = read_elr_el3();
		esr = read_esr_el3();
		far = read_far_el3();
	} else {
		ERROR("\tNeither in EL1 nor EL3.\n");
	}
	ERROR("\tSPSR_EL%d:\t0x%016lx\n", el, spsr);
	ERROR("\tELR_EL%d:\t0x%016lx\n", el, elr);
	ERROR("\tESR_EL%d:\t0x%016lx\n", el, esr);
	ERROR("\tFAR_EL%d:\t0x%016lx\n", el, far);
}

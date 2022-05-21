/*
 * BL32 Platform and Arch related code implemention for Alius LCS Domain.
 *
 * Maintainer: Liuwei <Wei.Liu@verisilicon.com>
 *
 * Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
 *
 */

#include <assert.h>
#include <platform_def.h>
#include <bl32/sp_min/platform_sp_min.h>

//For uart
#include <drivers/arm/pl011.h>

//For mmu
#include <lib/xlat_tables/xlat_tables_v2.h>
#define MAP_UART	MAP_REGION_FLAT(PL011_UART0_BASE,	\
					PL011_UART_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_FLASH	MAP_REGION_FLAT(BL_FLASH_BASE,	\
					BL_FLASH_SIZE,	\
					MT_MEMORY | MT_RO | MT_SECURE)

static const mmap_region_t plat_a15_mmap[] = {
	MAP_UART,
	MAP_FLASH,
	{0}
};

/* Now only config ram and mmap */
void a15_configure_mmu_svc_mon(unsigned long total_base,
					unsigned long total_size)
{
	/* define all sram to rw/memroy/secure */
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	/* re-define code sram to ro/memory/secure, because only ro can execute */
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	/* add others */
	mmap_add(plat_a15_mmap);
	init_xlat_tables();
	enable_mmu_svc_mon(0);
}

static console_t console;
static entry_point_info_t bl33_image_ep_info;

unsigned int plat_get_syscnt_freq2(void)
{
	return 1920000;
}


entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	console_pl011_register(PL011_UART0_BASE, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell SP_MIN where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = BL33_BASE;
	bl33_image_ep_info.spsr = 0x1d3;
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);
}

void sp_min_platform_setup(void)
{

}

void sp_min_plat_runtime_setup(void)
{

}

void sp_min_plat_arch_setup(void)
{
	a15_configure_mmu_svc_mon(BL_RAM_BASE, BL_RAM_SIZE);
}

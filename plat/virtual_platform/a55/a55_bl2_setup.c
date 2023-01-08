/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/bl_common.h>
#include <common/desc_image_load.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <platform_def.h>
//For uart
#include <uart_16550.h>

//For io_setup function
#include <a55_private.h>

//For mmu
#include <lib/xlat_tables/xlat_tables_v2.h>
#define MAP_UART	MAP_REGION_FLAT(PLAT_A55_UART_BASE,	\
					PLAT_A55_UART_SIZE,	\
					MT_DEVICE | MT_RW | MT_SECURE)

#define MAP_FLASH	MAP_REGION_FLAT(BL_FLASH_BASE,	\
					BL_FLASH_SIZE,	\
					MT_MEMORY | MT_RO | MT_SECURE)

static const mmap_region_t plat_a55_mmap[] = {
	MAP_UART,
	MAP_FLASH,
	{0}
};

/* Now only config ram and mmap */
void a55_configure_mmu_svc_mon(unsigned long total_base,
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
	mmap_add(plat_a55_mmap);
	init_xlat_tables();
	enable_mmu(0);
}

static console_t console;

/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
/*******************************************************************************
 * This function returns the list of loadable images.
 ******************************************************************************/
bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

/*******************************************************************************
 * This function flushes the data structures so that they are visible
 * in memory for the next BL image.
 ******************************************************************************/
void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

/*******************************************************************************
 * This function returns the list of executable images.
 ******************************************************************************/
bl_params_t *plat_get_next_bl_params(void)
{
	return get_next_bl_params_from_mem_params_desc();
}

void bl2_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	a55_io_setup();
}

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	console_16550_register(PLAT_A55_UART_BASE,
			PLAT_A55_UART_CLK_IN_HZ,
			PLAT_A55_BAUDRATE,
			&console);
	NOTICE("BL2: From bl1 arg0:0x%lx arg1:0x%lx arg2:0x%lx arg3:0x%lx\n", 
		arg0, arg1, arg2, arg3);
}

void bl2_plat_arch_setup(void)
{
	a55_configure_mmu_svc_mon(BL_RAM_BASE, BL_RAM_SIZE);
}

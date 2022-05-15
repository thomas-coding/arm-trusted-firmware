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
#include <drivers/arm/pl011.h>

//For io_setup function
#include <a15_private.h>

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
	a15_io_setup();
}

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	console_pl011_register(PL011_UART0_BASE, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);
	NOTICE("BL2: From bl1 arg0:0x%lx arg1:0x%lx arg2:0x%lx arg3:0x%lx\n", 
		arg0, arg1, arg2, arg3);
}

void bl2_plat_arch_setup(void)
{

}

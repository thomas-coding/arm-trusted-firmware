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

#include <assert.h>
#ifdef SPD_opteed
#include <lib/optee_utils.h>
#endif

//For uart
#include <uart_16550.h>

//For io_setup function
#include <a55_private.h>

static console_t console;

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl2_tzram_layout;
struct meminfo *bl2_plat_sec_mem_layout(void)
{
    return &bl2_tzram_layout;
}

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

	console_set_scope(&console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);

	/* Allow BL1 to see the whole Trusted RAM */
	bl2_tzram_layout.total_base = BL_RAM_BASE;
	bl2_tzram_layout.total_size = BL_RAM_SIZE;

	NOTICE("BL2: From bl1 arg0:0x%lx arg1:0x%lx arg2:0x%lx arg3:0x%lx\n", 
		arg0, arg1, arg2, arg3);
}

#if BL2_AT_EL3
void bl2_el3_plat_arch_setup(void)
{
	a55_configure_mmu_el3(bl2_tzram_layout.total_base,
				 bl2_tzram_layout.total_size,
				 BL_CODE_BASE, BL_CODE_END);
}
#else
void bl2_plat_arch_setup(void)
{
	a55_configure_mmu_el1(bl2_tzram_layout.total_base,
				 bl2_tzram_layout.total_size,
				 BL_CODE_BASE, BL_CODE_END);
}
#endif

static int a55_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);
	switch (image_id) {
		case BL32_IMAGE_ID:
		{
	#ifdef SPD_opteed
			bl_mem_params_node_t *pager_mem_params = NULL;
			bl_mem_params_node_t *paged_mem_params = NULL;

			pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
			assert(pager_mem_params);

			paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
			assert(paged_mem_params);

			err = parse_optee_header(&bl_mem_params->ep_info,
					&pager_mem_params->image_info,
					&paged_mem_params->image_info);
			if (err != 0)
				WARN("OPTEE header parse error.\n");
	#endif
			break;
		}
		default:
			/* Do nothing in default case */
			break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return a55_bl2_handle_post_image_load(image_id);
}

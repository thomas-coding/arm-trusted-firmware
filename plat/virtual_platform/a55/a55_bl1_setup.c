/*
 * Copyright (c) 2013-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <platform_def.h>

#include <bl1/bl1.h>
#include <common/tbbr/tbbr_img_def.h>

//For uart
#include <uart_16550.h>

//For mmu
#include <lib/xlat_tables/xlat_tables_v2.h>

//For io_setup function
#include <a55_private.h>

//For bl1_plat_get_image_desc function
#include <plat/common/platform.h>

//For mmu config
#include <a55_private.h>

static console_t console;

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;
struct meminfo *bl1_plat_sec_mem_layout(void)
{
    return &bl1_tzram_layout;
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_16550_register(PLAT_A55_UART_BASE,
			PLAT_A55_UART_CLK_IN_HZ,
			PLAT_A55_BAUDRATE,
			&console);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL_RAM_BASE;
	bl1_tzram_layout.total_size = BL_RAM_SIZE;
}

void bl1_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	a55_io_setup();
}

void bl1_plat_arch_setup(void)
{
	a55_configure_mmu_el3(bl1_tzram_layout.total_base, bl1_tzram_layout.total_size,
				BL_CODE_BASE, BL1_CODE_END);
}

/*
 * Default implementation for bl1_plat_handle_post_image_load(). This function
 * populates the default arguments to BL2. The BL2 memory layout structure
 * is allocated and the calculated layout is populated in arg1 to BL2.
 */
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	meminfo_t *bl2_secram_layout;
	meminfo_t *bl1_secram_layout;
	image_desc_t *image_desc;
	entry_point_info_t *ep_info;

	if (image_id != BL2_IMAGE_ID)
		return 0;

	/* Get the image descriptor */
	image_desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);
	assert(image_desc != NULL);

	/* Get the entry point info */
	ep_info = &image_desc->ep_info;

	/* Find out how much free trusted ram remains after BL1 load */
	bl1_secram_layout = bl1_plat_sec_mem_layout();

	/*
	 * Create a new layout of memory for BL2 as seen by BL1 i.e.
	 * tell it the amount of total and free memory available.
	 * This layout is created at the first free address visible
	 * to BL2. BL2 will read the memory layout before using its
	 * memory for other purposes.
	 */
	/* Used share memory as bl2 secure ram layout and delivery to BL2 through r1 register */
	bl2_secram_layout = (meminfo_t *)BL_SHARE_MEM_BASE;

	bl1_calc_bl2_mem_layout(bl1_secram_layout, bl2_secram_layout);

	/*
	 * There are some extra parameters for development such as boot mode
	 *  located at shared memory. So flush the whole shared memory.
	 */
	flush_dcache_range((uintptr_t)BL_SHARE_MEM_BASE, BL_SHARE_MEM_SIZE);

	ep_info->args.arg1 = (uintptr_t)bl2_secram_layout;

	VERBOSE("BL1: BL2 memory layout address = %p\n",
		(void *) bl2_secram_layout);
	return 0;
}




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
#include <drivers/arm/pl011.h>

//For mmu
#include <lib/xlat_tables/xlat_tables_v2.h>

//For io_setup function
#include <a15_private.h>

//For bl1_plat_get_image_desc function
#include <plat/common/platform.h>

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

static console_t console;

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;
struct meminfo *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/* Now only config ram and mmap */
void a15_configure_mmu_svc_mon(unsigned long total_base,
					unsigned long total_size)
{
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);

	mmap_add(plat_a15_mmap);
	init_xlat_tables();
	enable_mmu_svc_mon(0);
}

/*******************************************************************************
 * Perform any BL1 specific platform actions.
 ******************************************************************************/
void bl1_early_platform_setup(void)
{
	/* Initialize the console to provide early debug support */
	console_pl011_register(PL011_UART0_BASE, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL_RAM_BASE;
	bl1_tzram_layout.total_size = BL_RAM_SIZE;
}

void bl1_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	a15_io_setup();
}

void bl1_plat_arch_setup(void)
{
	a15_configure_mmu_svc_mon(bl1_tzram_layout.total_base,
				bl1_tzram_layout.total_size);
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




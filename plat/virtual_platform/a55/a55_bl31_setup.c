// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2020 VeriSilicon Holdings Co., Ltd.
 */

#include <assert.h>
#include <common/bl_common.h>
#include <common/desc_image_load.h>

#include <common/debug.h>
#include <drivers/console.h>
#include <platform_def.h>
//For uart
#include <uart_16550.h>

//For io_setup function
#include <a55_private.h>

//For gic
#include <plat_arm.h>

static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

static console_t console;

/*
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 */
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/*
	 * Check params passed from BL2
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (!bl33_image_ep_info.pc)
		panic();

	/* Initialize the console to provide early debug support */
	console_16550_register(PLAT_A55_UART_BASE,
			PLAT_A55_UART_CLK_IN_HZ,
			PLAT_A55_BAUDRATE,
			&console);

	console_set_scope(&console,
			CONSOLE_FLAG_BOOT | CONSOLE_FLAG_RUNTIME);
}

void bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();
}

/*
 * Perform the very early platform specific architectural setup here.
 */
void bl31_plat_arch_setup(void)
{
	a55_configure_mmu_el3(BL_RAM_BASE, BL_RAM_SIZE,
				BL_CODE_BASE, BL_CODE_END);
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info = NULL;

	assert(sec_state_is_valid(type));
	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

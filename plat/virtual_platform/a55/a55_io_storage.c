/*
 * IO storage settings for Alius platform.
 *
 * Maintainer: Liuwei <Wei.Liu@verisilicon.com>
 *
 * Copyright (C) 2021 VeriSilicon Microelectronics (Shanghai) Co., Ltd.
 *
 */

#include <assert.h>
#include <string.h>
#include <platform_def.h>
#include <common/debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_storage.h>
#include <drivers/io/io_semihosting.h>
#include <a55_private.h>
#include <common/bl_common.h>
#include <drivers/io/io_fip.h>
#include <drivers/io/io_memmap.h>

//For bl2 uuid
#include <tools_share/firmware_image_package.h>

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* IO devices */
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_block_spec_t bl2_uuid_spec = {
	.offset = BL2_BINARY_BASE,
	.length = BL2_BINARY_SIZE,
};

static const io_block_spec_t bl32_uuid_spec = {
	.offset = BL32_BINARY_BASE,
	.length = BL32_BINARY_SIZE,
};

/* By default, virtual platform platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	[BL2_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		NULL, //memory no need to check
	},
	[BL32_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		NULL, //memory no need to check
	},
};

void a55_io_setup(void)
{
	int io_result;

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == 0);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int result = 0;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];

	*image_spec = policy->image_spec;
	*dev_handle = *(policy->dev_handle);

	return result;
}

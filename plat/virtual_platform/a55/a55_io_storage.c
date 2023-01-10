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

static int check_fip(const uintptr_t spec);
static int check_memmap(const uintptr_t spec);

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

/* IO devices */
static const io_dev_connector_t *memmap_dev_con;
static uintptr_t memmap_dev_handle;

static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

static const io_block_spec_t mem_boot_spec = {
	.offset = PLAT_A55_FIP_MEM_BASE,
	.length = PLAT_A55_FIP_MEM_SIZE
};

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t bl31_uuid_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t bl32_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl32_extra2_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

/* By default, virtual platform platforms load images from the FIP */
static const struct plat_io_policy policies[] = {
	/* Fip binary load from memory */
	[FIP_IMAGE_ID] = {
		&memmap_dev_handle,
		(uintptr_t)&mem_boot_spec,
		check_memmap,
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		check_fip,
	},
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		check_fip,
	},
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		check_fip,
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		check_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		check_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		check_fip,
	},
};


static int check_fip(const uintptr_t spec)
{
	int result = 0;
	uintptr_t local_image_handle = 0;

	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}

	return result;
}

static int check_memmap(const uintptr_t spec)
{
	int result = 0;
	uintptr_t local_image_handle = 0;

	result = io_dev_init(memmap_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(memmap_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}

	return result;
}

void a55_io_setup(void)
{
	int io_result;

	/* Get connector and register device handle */
	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

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
	result = policy->check(policy->image_spec);

	if (result == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *(policy->dev_handle);
	} else {
		VERBOSE("Trying alternative IO\n");
	}

	return result;
}

/*
 * IO storage settings for a55 platform.
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
#ifndef DECRYPTION_SUPPORT_none
#include <drivers/io/io_encrypted.h>
#endif
#include <io_block.h>
#include <drivers/mmc.h>

//For bl2 uuid
#include <tools_share/firmware_image_package.h>

// for cert
#if SECURE_DEBUG
#include "plat_def_fip_uuid.h"
#include "secure_debug.h"
#endif

static int check_fip(const uintptr_t spec);
#ifdef BOOT_FROM_SD
static int check_sd(const uintptr_t spec);
#else
static int check_memmap(const uintptr_t spec);
#endif
#ifndef DECRYPTION_SUPPORT_none
static int check_enc_fip(const uintptr_t spec);
#endif

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

static const io_dev_connector_t *sd_dev_con;
static uintptr_t sd_dev_handle;

#ifndef DECRYPTION_SUPPORT_none
static const io_dev_connector_t *enc_dev_con;
static uintptr_t enc_dev_handle;
#endif

#ifdef BOOT_FROM_SD
static const io_block_spec_t sd_boot_spec = {
	.offset = PLAT_A55_IMAGE_SD_BASE,
	.length = PLAT_A55_IMAGE_SD_SIZE
};
#else
static const io_block_spec_t mem_boot_spec = {
	.offset = PLAT_A55_FIP_MEM_BASE,
	.length = PLAT_A55_FIP_MEM_SIZE
};
#endif

static unsigned char
block_buffer[PLAT_A55_BLK_BUF_SIZE] __aligned(PLAT_A55_BLK_BUF_SIZE);

static const io_block_dev_spec_t sd_dev_spec = {
	.buffer = {
		.offset	= (size_t)&block_buffer,
		.length	= PLAT_A55_BLK_BUF_SIZE,
	},
	.ops = {
		.read	= mmc_read_blocks,
		.write	= mmc_write_blocks,
	},
	.block_size	= MMC_BLOCK_SIZE,
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

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t tb_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FW_CERT,
};

static const io_uuid_spec_t trusted_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};
static const io_uuid_spec_t soc_fw_key_cert_uuid_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t tos_fw_key_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t nt_fw_key_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t soc_fw_cert_uuid_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t tos_fw_cert_uuid_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t nt_fw_cert_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif

#if SECURE_DEBUG
static const io_uuid_spec_t secure_debug_cert_uuid_spec = {
	.uuid = UUID_SECURE_DEBUG_CERT,
};
#endif

/* By default, virtual platform platforms load images from the FIP */
static struct plat_io_policy policies[] = {
	/* Fip binary load from memory */
	[FIP_IMAGE_ID] = {
		//set value later base on boot source
	},
#ifndef DECRYPTION_SUPPORT_none
	[ENC_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)NULL,
		check_fip
	},
#endif
#if ENCRYPT_BL2 && !defined(DECRYPTION_SUPPORT_none)
	[BL2_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		check_enc_fip,
	},
#else
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		check_fip,
	},
#endif
#if SECURE_DEBUG
	[SECURE_DEBUG_CERT_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&secure_debug_cert_uuid_spec,
		check_fip,
	},
#endif
#if ENCRYPT_BL31 && !defined(DECRYPTION_SUPPORT_none)
	[BL31_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		check_enc_fip,
	},
#else
	[BL31_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl31_uuid_spec,
		check_fip,
	},
#endif
#if ENCRYPT_BL32 && !defined(DECRYPTION_SUPPORT_none)
	[BL32_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		check_enc_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		check_enc_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&enc_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		check_enc_fip
	},
#else
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
#endif
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		check_fip,
	},
#if TRUSTED_BOARD_BOOT
	[TRUSTED_BOOT_FW_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tb_fw_cert_uuid_spec,
		check_fip
	},
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&trusted_key_cert_uuid_spec,
		check_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_key_cert_uuid_spec,
		check_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_key_cert_uuid_spec,
		check_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_key_cert_uuid_spec,
		check_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&soc_fw_cert_uuid_spec,
		check_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&tos_fw_cert_uuid_spec,
		check_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_cert_uuid_spec,
		check_fip
	},
#endif
};

static int check_fip(const uintptr_t spec)
{
	int result = 0;
	uintptr_t local_image_handle = 0;

	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0 && spec != (uintptr_t)NULL) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}

	return result;
}

#ifdef BOOT_FROM_SD
static int check_sd(const uintptr_t spec)
{
	int result = 0;
	uintptr_t local_image_handle = 0;

	result = io_dev_init(sd_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(sd_dev_handle, spec, &local_image_handle);
		if (result == 0)
			io_close(local_image_handle);
	}

	return result;
}
#else
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
#endif

#ifndef DECRYPTION_SUPPORT_none
static int check_enc_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if an encrypted FIP is available */
	result = io_dev_init(enc_dev_handle, (uintptr_t)ENC_IMAGE_ID);
	if (result == 0) {
		result = io_open(enc_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using encrypted FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}
#endif

void a55_io_setup(void)
{
	int io_result;

	/* Get connector and register device handle */
	io_result = register_io_dev_fip(&fip_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_memmap(&memmap_dev_con);
	assert(io_result == 0);

	io_result = register_io_dev_block(&sd_dev_con);
	assert(io_result == 0);

#ifndef DECRYPTION_SUPPORT_none
	io_result = register_io_dev_enc(&enc_dev_con);
	assert(io_result == 0);

	io_result = io_dev_open(enc_dev_con, (uintptr_t)NULL,
				&enc_dev_handle);
	assert(io_result == 0);
#endif

	/* Open connections to devices and cache the handles */
	io_result = io_dev_open(fip_dev_con, (uintptr_t)NULL,
				&fip_dev_handle);
	assert(io_result == 0);

	io_result = io_dev_open(memmap_dev_con, (uintptr_t)NULL,
				&memmap_dev_handle);
	assert(io_result == 0);

	io_result = io_dev_open(sd_dev_con, (uintptr_t)&sd_dev_spec,
				&sd_dev_handle);

	/* Ignore improbable errors in release builds */
	(void)io_result;
}

#ifdef BOOT_FROM_SD
static void a55_sd_boot(void)
{
	policies[FIP_IMAGE_ID].dev_handle = &sd_dev_handle;
	policies[FIP_IMAGE_ID].image_spec = (uintptr_t)&sd_boot_spec;
	policies[FIP_IMAGE_ID].check = check_sd;
}
#else
static void a55_memmap_boot(void)
{
	policies[FIP_IMAGE_ID].dev_handle = &memmap_dev_handle;
	policies[FIP_IMAGE_ID].image_spec = (uintptr_t)&mem_boot_spec;
	policies[FIP_IMAGE_ID].check = check_memmap;
}
#endif

void a55_boot_source(void)
{
#ifdef BOOT_FROM_SD
	NOTICE("boot from sd card\n");
	a55_sd_boot();
#else
	NOTICE("boot from memory\n");
	a55_memmap_boot();
#endif

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

	NOTICE("image id: %d\n", image_id);
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

/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <firmware_image_package.h>

#include "tbbr_config.h"
#include "plat_def_fip_uuid.h"

toc_entry_t plat_def_toc_entries[] = {
	{
		.name = "Secure Debug Certificate",
		.uuid = UUID_SECURE_DEBUG_CERT,
		.cmdline_name = "secure-debug-cert"
	}
};


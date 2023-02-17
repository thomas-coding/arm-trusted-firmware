/*
 * Copyright (c) 2014-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SECURE_DEBUG_H
#define SECURE_DEBUG_H

/* Secure debug reuse bl31 region */
#define SECURE_DEBUG_CERT_BASE			0x20100000
#define SECURE_DEBUG_CERT_SIZE			UL(1024 * 1024)
#define SECURE_DEBUG_CERT_LIMIT			(SECURE_DEBUG_CERT_BASE + SECURE_DEBUG_CERT_SIZE)

/* Define image id for io layer get method */
#define SECURE_DEBUG_CERT_IMAGE_ID			U(100)

/* TODO: now only support SHA 256 */
#define SD_HASH_LEN			32	/* SHA 256 */
#define SECURE_DEBUG_HASH_OID				"2.5.29.14"

#endif /* SECURE_DEBUG_H */

/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.global secure_debug_pk_hash
	.global secure_debug_pk_hash_end

	.section .rodata.a55_secure_debug_pk_hash, "a"

secure_debug_pk_hash:
	.incbin SECURE_DEBUG_PK_HASH
secure_debug_pk_hash_end:

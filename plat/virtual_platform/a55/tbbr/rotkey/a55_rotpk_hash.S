/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.global a55_rotpk_hash
	.global a55_rotpk_hash_end

	.section .rodata.a55_rotpk_hash, "a"

a55_rotpk_hash:
	.incbin A55_ROTPK_HASH
a55_rotpk_hash_end:

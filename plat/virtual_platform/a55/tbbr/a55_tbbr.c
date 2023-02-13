/*
 * Trusted Board Boot Requirement implemented on virtual platform a55 .
 * Ported from board_arm_trusted_boot.c
 *
 *
 */

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <lib/cassert.h>
#include <plat/common/platform.h>
#include <tools_share/tbbr_oid.h>
#include <drivers/auth/auth_mod.h>
#include <platform_def.h>
#include <plat/arm/common/plat_arm.h>

#define ROTPK_HASH_LEN			32	/* SHA 256 */

#if (ROTPK_HASH_LEN == 64) /* SHA512 */
/*
 * SEQUENCE (2 elem)
 *   SEQUENCE (2 elem)
 *     OBJECT IDENTIFIER 2.16.840.1.101.3.4.2.3 sha-512 (NIST Algorithm)
 *     NULL
 *   OCTET STRING (64 byte) 04 40 ...
 */
static const unsigned char rotpk_hash_hdr[] = \
	"\x30\x51" \
	"\x30\x0D" \
	"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x03" \
	"\x05\x00" \
	"\x04\x40";

#else /* SHA256*/
/*
 * SEQUENCE (2 elem)
 *   SEQUENCE (2 elem)
 *     OBJECT IDENTIFIER 2.16.840.1.101.3.4.2.1 sha-256 (NIST Algorithm)
 *     NULL
 *   OCTET STRING (32 byte) 04 20 ...
 */
static const unsigned char rotpk_hash_hdr[] = \
	"\x30\x31" \
	"\x30\x0D" \
	"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01" \
	"\x05\x00" \
	"\x04\x20";
#endif

static const unsigned int rotpk_hash_hdr_len = sizeof(rotpk_hash_hdr) - 1;
/* A buffer contains header of ROTPK hash DER and RAW of hash */
static unsigned char rotpk_hash_der[sizeof(rotpk_hash_hdr) - 1 + ROTPK_HASH_LEN] __aligned(CACHE_WRITEBACK_GRANULE);

/*
 * Return the non-volatile counter value stored in the platform. The cookie
 * will contain the OID of the counter in the certificate.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr)
{
	*nv_ctr = 11;
	return 0;
}

/*
 * Store a new non-volatile counter value. By default on ARM development
 * platforms, the non-volatile counters are RO and cannot be modified. We expect
 * the values in the certificates to always match the RO values so that this
 * function is never called.
 *
 * Return: 0 = success, Otherwise = error
 */
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr)
{
	return 1;
}

int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags)
{
	void *rotpk_raw_ptr;
	uint8_t *dst;

	/* Fill HASH header */
	memcpy(rotpk_hash_der, rotpk_hash_hdr, rotpk_hash_hdr_len);
	dst = &rotpk_hash_der[rotpk_hash_hdr_len];

	/* Fill HASH body */
	extern unsigned char a55_rotpk_hash[];
	rotpk_raw_ptr = a55_rotpk_hash;
	memcpy(dst, rotpk_raw_ptr, ROTPK_HASH_LEN);

	*key_len = ROTPK_HASH_LEN + rotpk_hash_hdr_len;
	*flags = ROTPK_IS_HASH;
	*key_ptr = rotpk_hash_der;
	return 0;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}

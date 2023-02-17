

#include "plat_def_fip_uuid.h"
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/auth/auth_mod.h>
#include <drivers/auth/crypto_mod.h>
#include <drivers/io/io_storage.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/platform.h>

#include "secure_debug.h"
#include "a55_private.h"

image_info_t img_info = {
	.h.version = VERSION_2,
	.image_base = SECURE_DEBUG_CERT_BASE,
	.image_max_size = SECURE_DEBUG_CERT_SIZE,
};

auth_param_type_desc_t cert_subject_pk = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_PUB_KEY, 0);
auth_param_type_desc_t cert_sig = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_SIG, 0);
auth_param_type_desc_t cert_sig_alg = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_SIG_ALG, 0);
auth_param_type_desc_t cert_raw_data = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_RAW_DATA, 0);
auth_param_type_desc_t cert_extension_hash_data = AUTH_PARAM_TYPE_DESC(
	AUTH_PARAM_HASH, SECURE_DEBUG_HASH_OID);

#if (SD_HASH_LEN == 64) /* SHA512 */
/*
 * SEQUENCE (2 elem)
 *   SEQUENCE (2 elem)
 *     OBJECT IDENTIFIER 2.16.840.1.101.3.4.2.3 sha-512 (NIST Algorithm)
 *     NULL
 *   OCTET STRING (64 byte) 04 40 ...
 */
static const unsigned char sd_hash_hdr[] = \
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
static const unsigned char sd_hash_hdr[] = \
	"\x30\x31" \
	"\x30\x0D" \
	"\x06\x09\x60\x86\x48\x01\x65\x03\x04\x02\x01" \
	"\x05\x00" \
	"\x04\x20";
#endif

static const unsigned int sd_hash_hdr_len = sizeof(sd_hash_hdr) - 1;
/* A buffer contains header of hash DER and RAW of hash */
static unsigned char sd_hash_der[sizeof(sd_hash_hdr) - 1 + SD_HASH_LEN] __aligned(CACHE_WRITEBACK_GRANULE);

int sd_get_pk_hash(void **key_ptr, unsigned int *key_len)
{
	void *sdpk_raw_ptr;
	uint8_t *dst;

	/* Fill HASH header */
	memcpy(sd_hash_der, sd_hash_hdr, sd_hash_hdr_len);
	dst = &sd_hash_der[sd_hash_hdr_len];

	/* Fill HASH body */
	/* TODO: Get public key hash from OTP in product */
	extern unsigned char secure_debug_pk_hash[];
	sdpk_raw_ptr = secure_debug_pk_hash;
	memcpy(dst, sdpk_raw_ptr, SD_HASH_LEN);

	*key_len = SD_HASH_LEN + sd_hash_hdr_len;
	*key_ptr = sd_hash_der;
	return 0;
}

uint8_t dummy_chip_id[16] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
};

int sd_get_chipid(void **chipid_ptr, unsigned int *chipid_len)
{
	/* TODO: get chipid form OTP */
	*chipid_len = sizeof(dummy_chip_id);
	*chipid_ptr = dummy_chip_id;
	return 0;
}

static int load_cert(void)
{
	int io_result;
	uintptr_t dev_handle;
	uintptr_t image_handle;
	uintptr_t image_spec;
	size_t image_size;
	size_t bytes_read;

	/* Obtain a reference to the image by querying the platform layer */
	io_result = plat_get_image_source(SECURE_DEBUG_CERT_IMAGE_ID, &dev_handle, &image_spec);
	if (io_result != 0) {
		INFO("Not get image id=%u (%i)\n",
			SECURE_DEBUG_CERT_IMAGE_ID, io_result);
		INFO("Maybe debug cert not exist in fip image\n");
		return io_result;
	}

	/* Attempt to access the image */
	io_result = io_open(dev_handle, image_spec, &image_handle);
	if (io_result != 0) {
		WARN("Failed to access image id=%u (%i)\n",
			SECURE_DEBUG_CERT_IMAGE_ID, io_result);
		return io_result;
	}

	INFO("Loading image id=%u at address 0x%lx\n",
		SECURE_DEBUG_CERT_IMAGE_ID, img_info.image_base);

	/* Find the size of the image */
	io_result = io_size(image_handle, &image_size);
	if ((io_result != 0) || (image_size == 0U)) {
		WARN("Failed to determine the size of the image id=%u (%i)\n",
			SECURE_DEBUG_CERT_IMAGE_ID, io_result);
		goto exit;
	}

	/* Check that the image size to load is within limit */
	if (image_size > img_info.image_max_size) {
		WARN("Image id=%u size out of bounds\n", SECURE_DEBUG_CERT_IMAGE_ID);
		io_result = -EFBIG;
		goto exit;
	}

	/*
	 * image_data->image_max_size is a uint32_t so image_size will always
	 * fit in image_data->image_size.
	 */
	img_info.image_size = (uint32_t)image_size;

	/* We have enough space so load the image now */
	io_result = io_read(image_handle, img_info.image_base, image_size, &bytes_read);
	if ((io_result != 0) || (bytes_read < image_size)) {
		WARN("Failed to load image id=%u (%i)\n", SECURE_DEBUG_CERT_IMAGE_ID, io_result);
		goto exit;
	}

	INFO("Image id=%u loaded: 0x%lx - 0x%lx\n", SECURE_DEBUG_CERT_IMAGE_ID, img_info.image_base,
	     (uintptr_t)(img_info.image_base + image_size));

exit:
	(void)io_close(image_handle);

	(void)io_dev_close(dev_handle);

	return io_result;
}

int verify_cert(void)
{
	int rc;
	void *data_ptr, *pk_ptr,  *sig_ptr, *sig_alg_ptr;
	unsigned int data_len, pk_len, sig_len, sig_alg_len;
	void *public_key_hash;
	unsigned int public_key_hash_len;
	char * cert_extension_hash;
	uint32_t cert_extension_hash_len;
	void *chipid;
	unsigned int chipid_len;

	/* Step 1: veiry certificate by public key from this certificate*/
	/* Check and prase cert, save temporary variables in auth mode */
	rc = img_parser_check_integrity(IMG_CERT, (void *)img_info.image_base,
		img_info.image_size);
	if (rc != 0) {
		WARN("Failed to check cert rc:%d \n", rc);
		return rc;
	}

	/* Get data ptr, refer to cert to be signid data */
	rc = img_parser_get_auth_param(IMG_CERT, &cert_raw_data,
			(void *)img_info.image_base,
			img_info.image_size, &data_ptr, &data_len);
	if (rc != 0) {
		WARN("Failed to get data rc:%d \n", rc);
		return rc;
	}

	/* Get the signature from current image */
	rc = img_parser_get_auth_param(IMG_CERT, &cert_sig,
			(void *)img_info.image_base, img_info.image_size, &sig_ptr, &sig_len);
	if (rc != 0) {
		WARN("Failed to get sig rc:%d \n", rc);
		return rc;
	}

	/* Get the signature algorithm from current image */
	rc = img_parser_get_auth_param(IMG_CERT, &cert_sig_alg,
			(void *)img_info.image_base, img_info.image_size, &sig_alg_ptr, &sig_alg_len);
	if (rc != 0) {
		WARN("Failed to get sig alg rc:%d \n", rc);
		return rc;
	}

	/* Get the signature algorithm from current image */
	rc = img_parser_get_auth_param(IMG_CERT, &cert_subject_pk,
			(void *)img_info.image_base, img_info.image_size, &pk_ptr, &pk_len);
	if (rc != 0) {
		WARN("Failed to get public key rc:%d \n", rc);
		return rc;
	}

	/* Ask the crypto module to verify the signature */
	rc = crypto_mod_verify_signature(data_ptr, data_len,
						sig_ptr, sig_len,
						sig_alg_ptr, sig_alg_len,
						pk_ptr, pk_len);	
	if (rc != 0) {
		WARN("signature verify fail rc:%d \n", rc);
		return rc;
	}

	/* Step 2: veiry public key from certificate*/
	/* Ask platform to get public key hash */
	rc = sd_get_pk_hash(&public_key_hash, &public_key_hash_len);
	if (rc != 0) {
		WARN("verify public key hash fail rc:%d \n", rc);
		return rc;
	}

	/* Ask the crypto-module to verify the key hash */
	rc = crypto_mod_verify_hash(pk_ptr, pk_len,
				    public_key_hash, public_key_hash_len);
	if (rc != 0) {
		WARN("verify public key hash fail rc:%d \n", rc);
		return rc;
	}

	/* Step 3: veiry extension hash from certificate*/
	/* Get the extension hash from certificate */
	rc = img_parser_get_auth_param(IMG_CERT, &cert_extension_hash_data,
			(void *)img_info.image_base, img_info.image_size, (void *)&cert_extension_hash, &cert_extension_hash_len);
	if (rc != 0) {
		WARN("Failed to get hash rc:%d \n", rc);
		return rc;
	}

	/* Remove tag and length */
	cert_extension_hash += 2;
	cert_extension_hash_len -= 2;
	
	/* Fill HASH header and body */
	memcpy(sd_hash_der, sd_hash_hdr, sd_hash_hdr_len);
	//unsigned char *dst = &sd_hash_der[sd_hash_hdr_len];
	memcpy(sd_hash_der + sd_hash_hdr_len, cert_extension_hash, SD_HASH_LEN);

	/* Get chipid */
	rc = sd_get_chipid(&chipid, &chipid_len);
	if (rc != 0) {
		WARN("get chipid fail rc:%d \n", rc);
		return rc;
	}

	/* Ask the crypto-module to verify the extension hash */
	rc = crypto_mod_verify_hash(chipid, chipid_len,
				    sd_hash_der, sd_hash_hdr_len + SD_HASH_LEN);
	if (rc != 0) {
		WARN("verify cert extension fail rc:%d \n", rc);
		return rc;
	}

	return rc;
}



int verify_debug_cert(void)
{
	int rc;

	rc = load_cert();
	if (rc != 0) {
		INFO("not load secure debug cert\n");
		return rc;
	}

	rc = verify_cert();
	if (rc != 0) {
		INFO("Verify secure debug cert fail\n");
		return rc;
	}

	INFO("Verify debug cert ok, enable debug\n");

	//TODO: enable secure debug here
	return rc;
}


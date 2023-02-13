#
# Compilation settings for virtual platform a55 TBBR.
#
#

# To disable AUTH when security is bypassed.
DYN_DISABLE_AUTH	:= 1
GENERATE_COT		:= 1

## Anti-Rollback version
# Currently, there is only one API to get the NVCTR_VAL from GSC.
# So TFW_NVCTR_VAL must same as NTFW_NVCTR_VAL. This can be fixed
# after BootRom according to application.
# GSC return counter is 10, suit for it.
TFW_NVCTR_VAL		?= 11
NTFW_NVCTR_VAL		?= 11

# Root key hash import to code
A55_ROTPK_HASH		:=	$(PLAT_PATH)/tbbr/rotkey/rsa_public_key_hash.bin
ROT_KEY						:=	$(PLAT_PATH)/tbbr/rotkey/rsa_private.pem
KEY_SIZE					:=  4096
$(eval $(call add_define_val,A55_ROTPK_HASH,'"$(A55_ROTPK_HASH)"'))

## TBBR source code
TBBR_SOURCES := ${PLAT_PATH}/tbbr/a55_tbbr.c			\
		${PLAT_PATH}/tbbr/rotkey/a55_rotpk_hash.S		\
		drivers/auth/auth_mod.c				\
		drivers/auth/crypto_mod.c			\
		drivers/auth/img_parser_mod.c			\
		drivers/auth/tbbr/tbbr_cot_common.c

## include x509 parser
include drivers/auth/mbedtls/mbedtls_x509.mk

## Choosing cryptographic engine
ifeq (${ESECURE_ENGINE_INTEG}, 1) # Use eSecure crypto engine
	include ${PLAT_PATH}/drivers/esecure/esecure_crypto.mk
else # Use mbedtls soft crypoto
	include drivers/auth/mbedtls/mbedtls_crypto.mk
endif

BL1_SOURCES += ${TBBR_SOURCES}					\
		drivers/auth/tbbr/tbbr_cot_bl1.c

BL2_SOURCES += ${TBBR_SOURCES}					\
		drivers/auth/tbbr/tbbr_cot_bl2.c

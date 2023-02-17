#
# Compilation settings for virtual platform a55 secure debug.
#
#

BL2_SOURCES += \
		${PLAT_PATH}/secure_debug/secure_debug_key/secure_debug_pk_hash.S		\
		${PLAT_PATH}/secure_debug/secure_debug.c

# Root key hash import to code
SECURE_DEBUG_PK_HASH		:=	$(PLAT_PATH)/secure_debug/secure_debug_key/secure_debug_pk_hash.bin
$(eval $(call add_define_val,SECURE_DEBUG_PK_HASH,'"$(SECURE_DEBUG_PK_HASH)"'))




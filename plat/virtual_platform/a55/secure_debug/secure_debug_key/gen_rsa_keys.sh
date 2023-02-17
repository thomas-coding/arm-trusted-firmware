# !/bin/sh
#
# A script to generate developmet secure debug key for a55 platform
#
# This scripts is based on plat\arm\board\common\protpk\README
# The key is only used to development.
#
#

HASH_ALG=sha256
KEY_SIZE=4096
KEY_NAME=secure_debug
PUBLIC_KEY_HASH_NAME=secure_debug_pk_hash

rm -f ${KEY_NAME}.pem
rm -f ${PUBLIC_KEY_HASH_NAME}.bin

echo "===========Generate RSA key ==========="
echo "Key algorithm    - RSA${KEY_SIZE}"
echo "Digest algorithm - ${HASH_ALG}"
echo

# Generate rot rot private key.
openssl genrsa ${KEY_SIZE} > ${KEY_NAME}.pem

# Generate rot public key hash
openssl rsa -in ${KEY_NAME}.pem -pubout -outform DER | \
openssl dgst -${HASH_ALG} -binary > ${PUBLIC_KEY_HASH_NAME}.bin

echo
echo "OUTPUT: "
echo "Private key       : ${KEY_NAME}.pem"
echo "Public key Digest : ${PUBLIC_KEY_HASH_NAME}.bin"
echo "========Generate RSA key Done========="

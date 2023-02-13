# !/bin/sh
#
# A script to generate developmet Root Keys for Alius platform.
#
# This scripts is based on plat\arm\board\common\protpk\README
# The key is only used to development.
#
#

HASH_ALG=sha256
KEY_SIZE=4096

rm -f rsa_private.pem
rm -f rsa_public_key_hash.bin

echo "===========Generate RSA RoTPK==========="
echo "Key algorithm    - RSA${KEY_SIZE}"
echo "Digest algorithm - ${HASH_ALG}"
echo

# Generate rot rot private key.
openssl genrsa ${KEY_SIZE} > rsa_private.pem

# Generate rot public key hash
openssl rsa -in rsa_private.pem -pubout -outform DER | \
openssl dgst -${HASH_ALG} -binary > rsa_public_key_hash.bin

echo
echo "OUTPUT: "
echo "Private key       : rsa_private.pem"
echo "Public key Digest : rsa_public_key_hash.bin"
echo "========Generate RSA RoTPK Done========="

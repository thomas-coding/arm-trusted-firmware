#!/bin/bash
# Usage:
# ./gen_cert.sh --chipid xxxx

# Define cert name
secure_debug_cert_name=secure_debug_cert

# Path
shell_path=$(cd "$(dirname "$0")" || exit;pwd)
atf_root_path=${shell_path}/../../../../..
fiptool_path=${atf_root_path}/tools/fiptool
fip_path=${atf_root_path}/build/a55/release
secure_debug_cert_name=secure_debug_cert


# Get chip id
if [[ $1  = "--chipid" ]]; then
    chipid=$2
else
    chipid=00112233445566778899aabbccddeeff
fi
echo ${chipid}

# Remove cert
rm -f ${secure_debug_cert_name}.crt

# Generate hash base on chipid
hash_get=`echo ${chipid} | xxd -r -ps | sha256sum`
chipid_hash=${hash_get:0:64}

# Generate cert
openssl req -new -x509 -days 365 -key secure_debug.pem -keyform PEM \
-outform DER -sha256 -sigopt rsa_padding_mode:pss \
-sigopt rsa_pss_saltlen:-1 -batch -out ${secure_debug_cert_name}.crt \
-extensions v3_req -addext subjectKeyIdentifier=$chipid_hash

if [ -e ${shell_path}/${secure_debug_cert_name}.crt ]; then
    echo "Generate certificate ok"
else
    echo "Fail to generate certificate, exit!!!!!!!!!!"
    exit
fi


# Add cert to fip image
if [ ! -e ${fip_path}/fip.bin ]; then
    echo "File ${fip_path}/fip.bin not exist, can't add cert to fip image!!!!!!!!!!"
    exit
fi
${fiptool_path}/fiptool update --secure-debug-cert ${secure_debug_cert_name}.crt ${fip_path}/fip.bin
echo "Add certificate to fip image ok"

# Dump fip image info to check
#${fiptool_path}/fiptool info ${fip_path}/fip.bin

# Remove cert
rm -f ${secure_debug_cert_name}.crt

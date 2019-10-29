#! /bin/bash

file=$1     #file name
path=$2     #script path

name_block_size=20        #max file name len
file_name_block="FFFFFFFFFFFFFFFFFFFF"
name=`echo ${file##*/}${file_name_block}`
echo -n ${name:0:${name_block_size}} >> ${file}

sk="76931FAC9DAB2B36C248B87D6AE33F9A62D7183A5D5789E4B2D6B441E2411DC7"     # ecc sk
echo "sk "${sk}

hash_out=$(sha256sum ${file})     #hash
hash=${hash_out%%" "*}
echo "hash "${hash}

signature_out=$(${path}/crypto -h ${hash} -s ${sk})
#echo ${signature_out}

signature=${signature_out##*"signature:"}
echo "signature "${signature}    # len = 129

echo -n ${signature:0:128} >> ${file}

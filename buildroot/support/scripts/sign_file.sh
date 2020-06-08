#! /bin/bash

if [ $# -lt 2 ] ; then
echo "Error param input ($#) !"
echo "Usage: $0 file_name file_path"
echo "file_name: The file name in FS"
echo "file_path: The file path in FS."
exit
fi

file=$1     #file name
path=$2     #script path

#check compile quite or not
if [ "$Q" == "@" ];then
quiet=1
else
quiet=0
fi

name_block_size=20        #max file name len
file_name_block="FFFFFFFFFFFFFFFFFFFF"
name=`echo ${file##*/}${file_name_block}`
echo -n ${name:0:${name_block_size}} >> ${file}

sk="76931FAC9DAB2B36C248B87D6AE33F9A62D7183A5D5789E4B2D6B441E2411DC7"     # ecc sk
if [ "$quiet" == "0" ];then
echo "sk "${sk}
fi

hash_out=$(sha256sum ${file})     #hash
hash=${hash_out%%" "*}
if [ "$quiet" == "0" ];then
echo "hash "${hash}
fi

signature_out=$(${path}/crypto -h ${hash} -s ${sk})
#echo ${signature_out}

signature=${signature_out##*"signature:"}
if [ "$quiet" == "0" ];then
echo "signature "${signature}    # len = 129
fi

hash=`echo $hash | tr 'a-z' 'A-Z'`

if [ $4 ] ; then
	CONFIG_FILE=$4
fi

if grep ^BR2_CFG_ENV_TYPE_QA=y $CONFIG_FILE > /dev/null ; then
signature=`${path}/crypto -r ${hash} ${path}/curl_staging.sh`
fi

if grep ^BR2_CFG_ENV_TYPE_STAGING=y $CONFIG_FILE > /dev/null ; then
signature=`${path}/crypto -r ${hash} ${path}/curl_staging.sh`
fi

if grep ^BR2_CFG_ENV_TYPE_PROD=y $CONFIG_FILE > /dev/null ; then
signature=`${path}/crypto -r ${hash} ${path}/curl_prod.sh`
fi

echo -n ${signature:0:128} >> ${file}

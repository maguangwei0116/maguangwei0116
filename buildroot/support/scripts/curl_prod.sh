#! /bin/bash

# for join buff
left_quot=\{
right_quot=\}

vault_token="X-Vault-Token:"
input=\"input\":\"$(cat)\"
token=""
all_input=""

token=`curl -s -l -k -H "Content-Type:application/json" -H "Data_Type:msg" -X POST \
	--data '{"role_id":"bad4f56b-f2d3-7083-d227-b5c107128700","secret_id":"a3cdb607-c145-1176-d0aa-5ec274f4b27a"}' \
	https://vault.redtea.io:8200/v1/auth/approle/login | awk -F '"' '{print $(NF-25)}'`

signature=`curl -s -l -k -H "Content-Type:application/json" -H "Data_Type:msg" \
	-H ${vault_token}${token} -X POST --data ${left_quot}${input}${right_quot} \
	https://vault.redtea.io:8200/v1/transit/sign/shareProfilePrivateKey/sha2-256 | \
	awk -F '"' '{print $(NF-7)}' | awk -F ':' '{print $3}'`

echo $signature


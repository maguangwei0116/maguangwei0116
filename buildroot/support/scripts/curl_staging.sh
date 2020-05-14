#! /bin/bash

# for join buff
left_quot=\{
right_quot=\}

vault_token="X-Vault-Token:"
input=\"input\":\"$(cat)\"
token=""
all_input=""

token=`curl -s -l -k -H "Content-Type:application/json" -H "Data_Type:msg" -X POST \
	--data '{"role_id":"f65bc2a2-ab95-ba68-45a0-edc0e0bfe164","secret_id":"47400ba2-cdd8-96db-94b6-fabed706e95f"}' \
	https://vault-qa.redtea.io:8200/v1/auth/approle/login | awk -F '"' '{print $(NF-25)}'`

signature=`curl -s -l -k -H "Content-Type:application/json" -H "Data_Type:msg" \
	-H ${vault_token}${token} -X POST --data ${left_quot}${input}${right_quot} \
	https://vault-qa.redtea.io:8200/v1/transit/sign/testPrivateKey/sha2-256 | \
	awk -F '"' '{print $(NF-7)}' | awk -F ':' '{print $3}'`

echo $signature
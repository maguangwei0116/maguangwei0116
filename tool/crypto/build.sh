#! /bin/bash

make all

cp -f crypto ../../buildroot/support/scripts/crypto
cp -f curl.sh ../../buildroot/support/scripts/curl.sh

make clean


#!/bin/sh
# Customer applications startup scirpt

echo "start oemapp!!"

cd /oemapp; ./start_redtea_app start &
cd -

echo "start oemapp!!!!!!!!!!!!!!"
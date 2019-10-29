
#! /bin/sh

WRAPPER_CONFIG=$3

echo "" > $WRAPPER_CONFIG

modules=`ls $2`

for d in $modules; do

c=$1`echo $d | tr a-z- A-Z_`
cat << EOF >> $WRAPPER_CONFIG
config $c
	bool "$d ($c)"
EOF

done

exit 0

#!/bin/bash

##########################################################
#
# sign.sh  tools/signature temp/rootfs 192.168.1.160 9600
#
##########################################################

SIGNTOOL=$1
folder=$2
HOST=$3
PORT=$4
BOARD=$5
VERSION=$6

STRIP=$7

CUR_YEAR=`date +'%Y'`
EXP_YEAR=`expr $CUR_YEAR + 20`
MONTH=`date +'%m'`
DAY=`date +'%d'`
CURRENT_TIME=$CUR_YEAR$MONTH$DAY
EXPIRED_TIME=$EXP_YEAR$MONTH$DAY


[ -z ${folder} ] && echo "Error: file path needed!" && exit 1

appcount=0
apkcount=0
jarcount=0
tacount=0
dllcount=0
drvcount=0
ttfcount=0
stripcount=0

all_file=`find ${folder}`
for i in ${all_file}
do
	if [ -d $i -o -L $i ] 
	then
		continue;
	fi
	
	filetype=`file $i`
	appname=`basename $i`
	
	echo "$filetype" | grep "shared object, ARM," 1>/dev/null
	if [ "$?" = "0" ]
	then
		${STRIP} --strip-all --strip-unneeded $i
		stripcount=`expr $stripcount + 1`
		echo "$appname" | grep ".so" 1>/dev/null
		if [ "$?" = "0" ]
		then
			echo "  Signature [$VERSION]  lib $i"
			$SIGNTOOL -q -g -k mf -t lib -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
			if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
			dllcount=`expr $dllcount + 1`
		else
			echo "  Signature [$VERSION]  app $i"
			$SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
 			if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
			appcount=`expr $appcount + 1`
		fi
		continue;
	fi
	
	echo "$filetype" | grep "relocatable, ARM" 1>/dev/null
	if [ "$?" = "0" ]
	then
		echo "  Signature [$VERSION]  drv $i"
		$SIGNTOOL -q -g -k mf -t drv -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		drvcount=`expr $drvcount + 1`
		continue;
	fi
	
	echo "$filetype" | grep "executable, ARM," 1>/dev/null
	if [ "$?" = "0" ]
	then
		echo "  Signature [$VERSION]  app $i"
		${STRIP} --strip-all --strip-unneeded $i
		stripcount=`expr $stripcount + 1`
		$SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		appcount=`expr $appcount + 1`
		continue;
	fi
	
	if [[ $appname =~ \AS0.apk$ ]]
	then
		echo "  Signature [$VERSION] apk $i with AS0"
		$SIGNTOOL -q -l -k AS0 -o AS0.pem -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		$SIGNTOOL -q -g -t ukey -V "NEW POS TECHNOLOGY LIMITED" -d $CURRENT_TIME -i 0 -e $EXPIRED_TIME -f AS0.pem -o AS0.puk -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; rm AS0.pem; fi
		cat $i AS0.puk > $i.tmp
		$SIGNTOOL -q -g -m "SIG:0002" -k AS0 -t app -f $i.tmp -o $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; rm AS0.pem AS0.puk $i.tmp; fi
		rm AS0.pem AS0.puk $i.tmp
		apkcount=`expr $apkcount + 1`
		continue;
	fi
	
	if [[ $appname =~ \.apk$ ]]
	then
		echo "  Signature [$VERSION]  apk $i"
		$SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		apkcount=`expr $apkcount + 1`
		continue;
	fi
	
	if [[ $appname =~ \.odex$ ]]
	then
		echo "  Signature [$VERSION]  apk $i"
		$SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		size=`stat -c "%s" $i`
		signsize=456
		filesize=`expr $size - $signsize`
		split -b $filesize $i
		mv xaa $i
		mv xab $i.sign
		apkcount=`expr $apkcount + 1`
		continue;
	fi
	if [[ $appname =~ \.jar$ ]] 
	then
		echo "  Signature [$VERSION]  jar $i"
		$SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
		if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		jarcount=`expr $jarcount + 1`
		continue;
	fi
        if [[ $appname =~ \.ta$ ]]
        then
                echo "  Signature [$VERSION]  ta $i"
                $SIGNTOOL -q -g -k mf -t app -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
                if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
		tacount=`expr $tacount + 1`
                continue;
        fi
        if [[ $appname == "DroidSansFallback.ttf" ]]
        then
                echo "  Signature [$VERSION]  ttf $i"
                $SIGNTOOL -q -g -k mf -t lib -f $i -a "$BOARD:$appname" -V "NEW POS TECHNOLOGY LIMITED" -v $VERSION -d $CURRENT_TIME -e $EXPIRED_TIME -c $HOST:$PORT
                if [ "$?" != "0" ]; then echo "FAIL"; exit 1; fi
                ttfcount=`expr $ttfcount + 1`
                continue;
        fi
done

echo "$drvcount DRV files were signed"
echo "$appcount APP files were signed"
echo "$apkcount APK files were signed"
echo "$jarcount JAR files were signed"
echo "$tacount TA files were signed"
echo "$dllcount DLL files were signed"
echo "$ttfcount TTF files were signed"
echo "$stripcount ELF files were striped"


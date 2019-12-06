#!/bin/bash

SrcDir=$1
OutDir=$2
UbinizeToolDir=$3
UbiFileName=$4
TargetVer=$5

if [ $# != 5 ]
then
	echo "ERROR: Error input param"
	echo "USAGE: $0 SrcDir OutDir UbinizeToolDir UbiFileName TargetVer"
	echo " e.g.: $0 ./oemapp ./target ./tools oemapp.ubi V0.0.1"
	exit 1;
fi

#check compile quite or not
if [ "$Q" == "@" ];then
quiet=1
else
quiet=0
fi

if [ "$quiet" == "0" ]
then
	echo "ubinize release target: $UbiFileName version: $TargetVer"
fi

if [ ! -d $SrcDir ]
then
    echo "The $SrcDir is not exist"
    exit 1
fi

if [ ! -d $OutDir ]
then
    mkdir -p $OutDir
    if [ ! -d $OutDir ]
    then
	echo "Can not create $OutDir directory"
	exit 1
    fi
fi

if [ -z $TargetVer ]
then
	echo "TargetVer: $TargetVer ..."
    echo "can not found the version"
    exit 1
fi

# Remove old targets
if [ -e $OutDir/oemapp.squashfs ]
then
	rm -rf $OutDir/oemapp.squashfs 
fi
if [ -e $OutDir/$UbiFileName ]
then
	rm -rf $OutDir/$UbiFileName
fi

# Make new xxx.squashfs
if [ "$quiet" == "0" ]
then
echo "mksquashfs $SrcDir $OutDir/oemapp.squashfs"
mksquashfs $SrcDir $OutDir/oemapp.squashfs
else
mksquashfs $SrcDir $OutDir/oemapp.squashfs >/dev/null 2>&1
fi

if [ ! -e $OutDir/oemapp.squashfs ]
then
    echo "Can not create oemapp squashfs"
    exit 1
fi

if [ ! -e $UbinizeToolDir/ubinize ]
then
    echo "the $UbinizeTool is not exist"
    exit 1
fi

# Make new xxx.ubi
chmod +x $UbinizeToolDir/ubinize
if [ "$quiet" == "0" ]
then
echo "$UbinizeToolDir/ubinize -o $OutDir/$UbiFileName -m 2048 -p 128KiB -s 2048 $OutDir/ubinize_oemapp_ubi.cfg"
$UbinizeToolDir/ubinize -o $OutDir/$UbiFileName -m 2048 -p 128KiB -s 2048 $OutDir/ubinize_oemapp_ubi.cfg
else
$UbinizeToolDir/ubinize -o $OutDir/$UbiFileName -m 2048 -p 128KiB -s 2048 $OutDir/ubinize_oemapp_ubi.cfg > /dev/null 2>&1
fi
if [ ! -e $OutDir/$UbiFileName ]
then
    echo "Can not create oemapp ubifs"
    exit 1
fi

exit 0
   


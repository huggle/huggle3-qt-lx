#!/bin/sh

if [ "$#" -lt 1 ];then
    echo "Enter version"
    read v
else
    v="$1"
fi

tar -zcf "huggle_"$v".orig.tar.gz" huggle
cp -r huggle huggle-"$v"
cd huggle-"$v"
debuild -us -uc

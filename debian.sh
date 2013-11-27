#!/bin/sh

echo "Enter version"
read v

tar -zcf "huggle_"$v".orig.tar.gz" huggle
mv huggle huggle-"$v"
cd huggle-"$v"
debuild -us -uc

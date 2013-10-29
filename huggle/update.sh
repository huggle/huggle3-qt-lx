#!/bin/sh
if [ -d ../.git ];then
  c=`git rev-list HEAD --count`
  hash=`git describe --always`
else
  c="production build"
  hash=""
fi
echo "build: $c $hash" > version.txt

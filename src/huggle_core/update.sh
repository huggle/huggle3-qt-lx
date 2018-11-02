#!/bin/sh

# This file generates version.txt which is built into resulting binary and is used
# to determine Huggle's actual version, including GIT revision number

# In case this is not a git repo, git string is replaced simply with "production build"
# these are remnants of past when production builds were running automatically from
# WMF labs

# This file is called from multiple places, most notably configure script

if [ -d ../../.git ];then
  c=`git rev-list HEAD --count`
  hash=`git describe --always --tags`
else
  c="production build"
  hash=""
fi
echo "build: $c $hash" > version.txt

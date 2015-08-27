#!/bin/sh


extensions='extension-thanks extension-review extension-splitter-helper mass-delivery enwiki extension-scoring extension-mass-delete'
orig=`pwd`

for xx in $extensions
do
    if [ -d "extension_list/$xx" ];then
       if [ ! -d "extension_list/$xx/src" ];then
           cd "extension_list/$xx/" || exit 1
           git submodule init
           git submodule update
           cd - || exit 1
       fi
    fi
done

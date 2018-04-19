#!/bin/sh

cd ../src || exit 1
echo "Preparing in `pwd`"
git submodule init
git submodule update
cd huggle_core || exit 1
sh update.sh
cp definitions_prod.hpp definitions.hpp || exit 1

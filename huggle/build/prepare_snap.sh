#!/bin/sh

git submodule init
git submodule update
sh update.sh
cp definitions_prod.hpp definitions.hpp
rm -rf "huggle_release"

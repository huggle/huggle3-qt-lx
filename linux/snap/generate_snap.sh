#!/bin/sh

source=../../huggle

echo "I will look for source in $source"

if [ ! -d "$source" ]
then
    echo "Error there is no source folder"
    exit 1
fi

mkdir tmp || exit 1
cd tmp || exit 1
cp ../snapcraft.yaml .
mkdir prime
mkdir prime/usr
mkdir prime/usr/lib
mkdir prime/lib
mkdir -p prime/usr/share/huggle/extensions
mkdir prime/usr/bin
cp -r "$source" src
rm -rf src/huggle_release
cd src || exit 1
rm -rf version.txt definitions.hpp
cp definitions_prod.hpp definitions.hpp
sh update.sh
cd .. || exit 1
snapcraft

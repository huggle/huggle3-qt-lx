#!/bin/sh

source=../../huggle

echo "I will look for source in $source"

if [ ! -d "$source" ]
then
    echo "Error there is no source folder"
    exit 1
fi

cd "$source" || exit 1
sh update.sh
cd - || exit 1
mkdir tmp || exit 1
cd tmp || exit 1
cp ../snapcraft.yaml .
mkdir prime
mkdir prime/bin
mkdir prime/usr
mkdir prime/usr/lib
mkdir -p snap/gui
mkdir -p prime/usr/share/huggle
mkdir prime/lib
mkdir -p prime/usr/share/huggle/extensions
mkdir prime/usr/bin
cp -r "../$source" src
rm -rf src/huggle_release
cp src/Resources/huggle3_newlogo.png prime/usr/share/huggle/huggle3_newlogo.png
cp src/build/huggle.desktop snap/gui/
# Now this is a hack that fixes problem with lookup of Qt libraries, if you know about better one, improve it!!
ln -s ../usr/lib/x86_64-linux-gnu/qt5/plugins/platforms prime/bin/platforms
cd src || exit 1
rm -rf definitions.hpp
cp definitions_prod.hpp definitions.hpp
cd .. || exit 1
snapcraft

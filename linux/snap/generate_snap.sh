#!/bin/sh

source=../../huggle/huggle_release

echo "I will look for binaries in $source"

if [ ! -d "$source" ]
then
    echo "Error there is no release folder, did you build huggle yet?"
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
cp -v "../$source/huggle" prime/usr/bin
cp -v ../$source/*.so prime/usr/lib/
cp -v ../$source/lib*/*.so prime/usr/lib
cp -v ../$source/extension_list/*/*.so prime/usr/share/huggle/extensions
echo "Automatically resolving and uploading all libs needed by huggle"
libs=`ldd prime/usr/bin/huggle | sed -e 's/.*=> //' -e 's/ .*//'`
for lib in $libs
do
#    cp -v --parents "$lib" "prime"
     cp -v "$lib" "prime/lib"
done
snapcraft

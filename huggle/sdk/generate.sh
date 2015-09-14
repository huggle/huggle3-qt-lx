#!/bin/sh

if [ -d release ];then
    echo "release exists, removing"
    rm -rv release
fi
folder="$HOME/huggle_sdk"
echo "This script will compile and install huggle SDK, default folder is $folder, press enter if you are fine with that, or type a different path"
read response
if [ x"$response" != x ];then
    folder="$response"
fi
if [ -d "$folder" ];then
    echo "$folder already exists!"
    exit 1
fi
mkdir release || exit 1
mkdir "$folder" || exit 1
cd .. || exit 1
./update.sh || exit 1
cd sdk/release || exit 1
cmake .. || exit 1
make || exit 1
cd .. || exit 1
mkdir "$folder/include" || exit 1
mkdir "$folder/lib" || exit 1
for file in `cat sources.list`
do
    header=`echo "$file" | sed 's/\.cpp$/.hpp/'`
    cp -v "$header" "$folder/include"
done
cp -v release/*.so "$folder/lib"
cp ../definitions.hpp release || exit 1
echo "#define HUGGLE_SDK" >> release/definitions.hpp
cp -v release/definitions.hpp "$folder/include"

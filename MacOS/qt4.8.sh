#!/bin/sh
if [ x"$1" != x ];then
  QTDIR="$1"
else
  QTDIR=/usr
fi
qtver="--qt4"
echo "Checking sanity of system..."
of=`pwd`
if [ -d huggle_release ];then
    echo "Release folder is already in folder, you need to delete it"
    exit 1
fi
cd ../huggle || exit 1
./configure --qtpath "$QTDIR" "$qtver" || exit 1
cd huggle_release || exit 1
make || exit 1
cd "$of"
cp -r ../huggle/huggle_release "$of/huggle_release" || exit 1
mkdir package || exit 1
mkdir package/huggle.app
cd package/huggle.app || exit 1
mkdir Contents
mkdir Contents/Frameworks
mkdir Contents/MacOS
mkdir Contents/PlugIns
mkdir Contents/Resources
mkdir Contents/SharedFrameworks
cd "$of"

echo "Copying the binaries to package"
cp info.plist package/huggle.app/Contents || exit 1
cp huggle_release/huggle package/huggle.app/Contents/MacOS || exit 1
cp ../huggle/Resources/huggle.icns package/huggle.app/Contents/Resources || exit 1
cd package
$QTDIR/bin/macdeployqt huggle.app -dmg

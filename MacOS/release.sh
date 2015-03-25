#!/bin/sh
QTDIR=~/Qt5.4.1/5.4/clang_64/
echo "Checking sanity of system..."
of=`pwd`
if [ -d huggle_release ];then
    echo "Release folder is already in folder, you need to delete it"
    exit 1
fi
cd ../huggle || exit 1
./configure --qtpath ~/Qt5.4.1/5.4/clang_64/ --qt5 --extension || exit 1
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
cp huggle_release/*.dylib package/huggle.app/Contents/MacOS || exit 1
cp huggle_release/huggle package/huggle.app/Contents/MacOS || exit 1
for extension in `ls huggle_release/extension_list`
do
    cp huggle_release/extension_list/$extension/*.dylib package/huggle.app/Contents/PlugIns/ || exit 1
done
cp ../huggle/Resources/huggle.icns package/huggle.app/Contents/Resources || exit 1
#cp -R $QTDIR/lib/QtWebKitWidgets.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtWebKit.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtNetwork.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtXml.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtWidgets.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtCore.framework/ package/huggle.app/Contents/Frameworks || exit 1
#cp -R $QTDIR/lib/QtGui.framework/ package/huggle.app/Contents/Frameworks || exit 1
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtWebKitWidgets.framework/Versions/5/QtWebKitWidgets (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtWebKit.framework/Versions/5/QtWebKit (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtXml.framework/Versions/5/QtXml (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.4.0, current version 5.4.1)
#	/Users/petanb/Qt5.4.1/5.4/clang_64/lib/QtCore.framework/Versions/5/QtCore
cd package
#install-name_tool -id @executable_path/../Frameworks/QtWebKitWidgets.framework/Versions/5/QtWebKitWidgets huggle.app/Contents/Frameworks/QtWebKitWidgets.framework/Versions/5/QtWebKitWidgets
#install-name_tool -id @executable_path/../Frameworks/QtWebKit.framework/Versions/5/QtWebKit huggle.app/Contents/Frameworks/QtWebKit.framework/Versions/5/QtWebKit
#install-name_tool -id @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork huggle.app/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork
#install-name_tool -id @executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml huggle.app/Contents/Frameworks/QtXml.framework/Versions/5/QtXml
#install-name_tool -id @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets huggle.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets
#install-name_tool -id @executable_path/../Frameworks/ huggle.app/Contents/Frameworks/
#install-name_tool -id @executable_path/../Frameworks/ huggle.app/Contents/Frameworks/
$QTDIR/bin/macdeployqt huggle.app -dmg

Windows
=========

This is used to build packages for windows, how-to:

*Open Huggle.nsi and update the version of huggle to current one
* Run powershell script release.ps1
* You can pick generator using -cmake_generator

By default this script will try to use Visual Studio to compile huggle, you may want to pass parameter -mingw true

We use MingW for 32bit version of huggle for release builds because it has fewer dependencies than Visual Studio and works on more versions of Windows with no need to install any redistributable files from MS

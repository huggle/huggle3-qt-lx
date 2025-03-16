Windows
=========

This is used to build packages for Windows systems.

# Instructions:

* Open `Huggle.nsi` and update the version of Huggle to the current one
* Run the PowerShell script `release.ps1`
* You can pick a generator using `-cmake_generator`

By default, this script will try to use Visual Studio to compile Huggle. You may want to pass the parameter `-mingw true`.

We use MinGW for the 32-bit version of Huggle release builds because it has fewer dependencies than Visual Studio and works on more versions of Windows with no need to install any redistributable files from Microsoft.

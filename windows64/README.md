Windows
=========

This is used to build the x64 version Huggle for Windows.

# Instructions:

* Download http://nsis.sourceforge.net/Download
* Install

It can be tricky, you will need to do this:

* Download VS 2013 redistributables and save them in this folder as `vcredist_x64_2013.exe` - this is necessary for OpenSSL to work
* Download VS 2015 or 2017 (depends what you are using to build Huggle) redistributables and save them in this folder as `vcredist_x64.exe`
* Download OpenSSL libraries for slproweb, 1.0.2x branch
* Open `pack.ps1` and update all of the variables, provide path to your VS, Qt and OpenSSL
* Install git and cmake (so that it works in PowerShell)
* Execute PowerShell `pack.ps1`

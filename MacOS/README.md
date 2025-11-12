This folder is used to package Huggle for MacOS.

Requirements:
* xcode
* git

Run `release.sh` and the `.dmg` file will be produced.  
By default the script builds a universal (arm64 + x86_64) binary; set the `HUGGLE_MACOS_ARCHS`
environment variable (for example `HUGGLE_MACOS_ARCHS=x86_64`) if you need to target a specific
architecture instead.

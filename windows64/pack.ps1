﻿# This just run the original script

# NOTE: if you decide to execute this from VS, you need to add this PATH:
# PATH=%PATH%;$(LocalDebuggerWorkingDirectory)..\huggle_core\Debug;$(LocalDebuggerWorkingDirectory)..\huggle_ui\Debug;$(LocalDebuggerWorkingDirectory)..\huggle_l10n\Debug;$(LocalDebuggerWorkingDirectory)..\huggle_res\Debug;$(LocalDebuggerWorkingDirectory)..\Debug;D:\libs\Qt\5.10.1\msvc2015_64\bin

# Qt5
# ..\windows\release.ps1 -cmake_generator "Visual Studio 15 2017 Win64" -vcinstall_path "E:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\" -msbuild_path "E:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt5_path "E:\Qt\Qt5.12.12\5.12.12\msvc2017_64" -openssl_path "E:\OpenSSL-Win64"

# Qt6
# ..\windows\release.ps1 -cmake_generator "Visual Studio 16 2019" -vcinstall_path "E:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\" -qt_version 6 -msbuild_path "E:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt_path "E:\Qt\6.5.3\msvc2019_64" -openssl_path "E:\OpenSSL-Win64"

# cmake prefix path: E:\Qt\6.6.2\msvc2019_64\lib\cmake

# This just run the original script
..\windows\release.ps1 -cmake_generator "Visual Studio 14 2015 Win64" -vcinstall_path "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\" -msbuild_path "C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt_path "D:\libs\Qt\5.10.1\msvc2015_64\" -openssl_path "D:\libs\OpenSSL-Win64_v1.0.2r"

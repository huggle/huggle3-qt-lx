# This just run the original script

# Qt5
# ..\windows\release.ps1 -cmake_generator "Visual Studio 15 2017 Win64" -vcinstall_path "E:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\" -msbuild_path "E:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt5_path "E:\Qt\Qt5.12.12\5.12.12\msvc2017_64" -openssl_path "E:\OpenSSL-Win64"

# Qt6
..\windows\release.ps1 -cmake_generator "Visual Studio 16 2019" -vcinstall_path "E:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\" -qt_version 6 -msbuild_path "E:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt_path "E:\Qt\6.5.3\msvc2019_64" -openssl_path "E:\OpenSSL-Win64"

# cmake prefix path: E:\Qt\6.6.2\msvc2019_64\lib\cmake
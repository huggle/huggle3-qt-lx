# This just run the original script
..\windows\release.ps1 -cmake_generator "Visual Studio 14 2015 Win64" -vcinstall_path "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\" -msbuild_path "C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt5_path "D:\libs\Qt\5.10.0\msvc2015_64\" -openssl_path "C:\OpenSSL-Win64"

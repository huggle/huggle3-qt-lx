# See ../windows/pack.ps1 for explanation of these scripts

$ErrorView='NormalView'

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

cmd /c """$vcvars"" && set" | ForEach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
    }
}

..\windows\release.ps1 -cmake_generator "Visual Studio 16 2019" -platform "Win64" -vcinstall_path "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" -qt_version 5 -msbuild_path "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" -vcredist vcredist_x64.exe -python $false -qt_path "C:\Qt\5.15.2\msvc2019_64" -openssl_path "H:\OpenSSL-Win64"
 


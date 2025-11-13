# This is a helper script for MS Windows that launches the release script sourcing MSVC variables, it can be used
# even if you don't have full VS installed, you just need the build tools that can be obtained using
#
# Run in admin PS
#
# winget install --id Microsoft.VisualStudio.2019.BuildTools -e --source winget
#
# Install full C++ stack
# & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vs_installer.exe" modify `
#    --installPath "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" `
#    --add Microsoft.VisualStudio.Workload.VCTools `
#    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
#    --add Microsoft.VisualStudio.Component.VC.CMake.Project `
#    --add Microsoft.VisualStudio.Component.VC.Redist.14.Latest `
#    --add Microsoft.VisualStudio.Component.Windows10SDK.19041 `
#    --includeRecommended --passive --norestart
#
# Get SSL (remove --x86 for 64 bit) - these will install into program files, this version is for Qt 5.15.2 only
#
# choco install openssl -y --version=1.1.1.1500 --x86

$ErrorView='NormalView'

$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars32.bat"

cmd /c """$vcvars"" && set" | ForEach-Object {
    if ($_ -match "^(.*?)=(.*)$") {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2])
    }
}

.\release.ps1 -cmake_generator "Visual Studio 16 2019" -vcinstall_path "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" -qt_version 5 -msbuild_path "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe" -python $false -qt_path "C:\Qt\5.15.2\msvc2019" -openssl_path "H:\OpenSSL-Win32"
 


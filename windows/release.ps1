# This is a powershell script that prepare huggle project so that it can be built

#  Copyright (c) 2015, Petr Bena
#  All rights reserved.

#  Redistribution and use in source and binary forms, with
#  or without modification, are permitted provided that
#  the following conditions are met:

#  1. Redistributions of source code must retain
#     the above copyright notice, this list 
#     of conditions and the following disclaimer.

#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the distribution.

#  3. Neither the name of Huggle nor the names of its contributors may be used
#     to endorse or promote products derived from this software without specific
#     prior written permission.

#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
#  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
#  THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
#  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

param
(
    [string]$msbuild_path = "C:\Program Files (x86)\MSBuild\12.0\Bin\MSBuild.exe",
    [string]$root_path = $PWD,
    [string]$qt5_path = "C:\Qt\5.4\msvc2013\",
    [string]$nsis_path = "C:\Program Files (x86)\NSIS\makensis.exe",
    [string]$openssl_path = "C:\OpenSSL-Win32",
    [string]$cmake_generator = "Visual Studio 12 2013",
    [bool]$mingw = $false,
    [string]$mingw_path = "C:\Qt\Tools\mingw491_32",
    [bool]$python = $true,
    [string]$vcredist = "vcredist_x86.exe"
)

$ErrorActionPreference = "Stop"

function PackageTest
{
    param
    (
        [parameter(Mandatory=$true)]
        [ValidateNotNullOrEmpty()]
        [string] $PackageName,

        [parameter(Mandatory=$true)]      
        [ValidateNotNullOrEmpty()]
        [string] $PackageUrl,

        [parameter(Mandatory=$true)]
        [string] $VariableName
    )

    Write-Host "Looking for $PackageName...    " -NoNewLine
    if (!(Test-Path $PackageUrl))
    {
        echo "ERROR"
        echo "Unable to find $PackageName at $PackageUrl, you can set the alternative path using -$VariableName=path"
        exit 1
    }
    echo ("OK");
}

Write-Host "Checking paths...     " -NoNewline

if (!(Test-Path "gpl.txt"))
{
    echo "ERROR"
    echo "Unable to find license, are you in right folder?"
    exit 1
}

if (!(Test-Path "../huggle/configure.ps1"))
{
    echo "ERROR"
    echo "This isn't a huggle windows folder, you need to run this script from within the ROOT/windows folder"
    exit 1
}

if ((Test-Path ".\build"))
{
    echo "ERROR"
    echo "The build folder is already present, please remove it first"
    exit 1
}

if ((Test-Path ".\release"))
{
    echo "ERROR"
    echo "The release folder is already present, please remove it first"
    exit 1
}

if (!(Test-Path $vcredist))
{
    echo "ERROR"
    echo "There is no $vcredist package, please download one from http://www.microsoft.com/en-us/download/details.aspx?id=40784 and place it in this folder"
    exit 1
}

echo "OK"

if ($mingw)
{
    PackageTest "MingW" "$mingw_path" "mingw_path"
} else
{
    PackageTest "MSBuild" "$msbuild_path" "msbuild_path"
}
PackageTest "Qt5" "$qt5_path" "qt5_path"
PackageTest "OpenSSL" "$openssl_path" "openssl_path"
PackageTest "nsis" "$nsis_path" "nsis_path"
$git_enabled = $true
Write-Host "Looking for git...    " -NoNewline
if (!(Get-Command git -errorAction SilentlyContinue))
{
    echo "NOT FOUND"
    $git_enabled = $false
    exit 1
}
echo "OK"
Write-Host "Looking for cmake...    " -NoNewline
if (!(Get-Command cmake -errorAction SilentlyContinue))
{
    echo "ERROR"
    echo "Unable to find cmake powershell snippet in this system"
    exit 1
}
echo "OK"

echo "Configuring the project..."

cd ..\huggle
if ($git_enabled -and (Test-Path("..\.git")))
{
    $rev_list = git rev-list HEAD --count | Out-String
    $rev_list = $rev_list.Replace("`n", "").Replace("`r", "")
    $hash = git describe --always --tags | Out-String
    $hash = $hash.Replace("`n", "").Replace("`r", "")
    echo "build: $rev_list $hash" > version.txt
} else
{
    echo "build: non-git build (windows)" > version.txt
}

#let's try to invoke cmake now
cd $root_path
echo "Running cmake"
mkdir build | Out-Null
cd build
if ($python)
{
    cmake ..\..\huggle\ -G "$cmake_generator" -DPYTHON_BUILD=true -DCMAKE_PREFIX_PATH:STRING=$qt5_path -Wno-dev=true -DHUGGLE_EXT=true -DQT5_BUILD=true
} else
{
    cmake ..\..\huggle\ -G "$cmake_generator" -DPYTHON_BUILD=false -DCMAKE_PREFIX_PATH:STRING=$qt5_path -Wno-dev=true -DHUGGLE_EXT=true -DQT5_BUILD=true
}
& $msbuild_path "huggle.sln" "/p:Configuration=Release" "/v:minimal"
cd $root_path
echo "Preparing the package structure"
mkdir release | Out-Null
mkdir release\deps | Out-Null
mkdir release\platforms | Out-Null
cp .\build\extension_list\enwiki\Release\huggle_en.dll release
cp .\build\extension_list\extension-thanks\Release\huggle_thanks.dll release
cp .\build\extension_list\extension-splitter-helper\Release\huggle_sh.dll release
cp .\build\extension_list\mass-delivery\Release\huggle_md.dll release
cp .\build\Release\core.dll release
cp .\build\Release\core.lib release
cp .\build\Release\huggle.exe release
if ($python)
{
    cp .\build\Release\py_hug.exe release
}
# get the qt
cp $qt5_path\plugins\platforms\qminimal.dll release\platforms
cp $qt5_path\plugins\platforms\qoffscreen.dll release\platforms
cp $qt5_path\plugins\platforms\qwindows.dll release\platforms
cp $qt5_path\bin\Enginio.dll release\deps
cp $qt5_path\bin\icudt53.dll release\deps
cp $qt5_path\bin\icuin53.dll release\deps
cp $qt5_path\bin\Qt5Bluetooth.dll release\deps
cp $qt5_path\bin\icuuc53.dll release\deps
cp $qt5_path\bin\Qt5CLucene.dll release\deps
cp $qt5_path\bin\Qt5Concurrent.dll release\deps
cp $qt5_path\bin\Qt5Core.dll release\deps
cp $qt5_path\bin\Qt5Declarative.dll release\deps
cp $qt5_path\bin\Qt5Designer.dll release\deps
cp $qt5_path\bin\Qt5DesignerComponents.dll release\deps
cp $qt5_path\bin\Qt5Gui.dll release\deps
cp $qt5_path\bin\Qt5Help.dll release\deps
cp $qt5_path\bin\Qt5Location.dll release\deps
cp $qt5_path\bin\Qt5Multimedia.dll release\deps
cp $qt5_path\bin\Qt5MultimediaQuick_p.dll release\deps
cp $qt5_path\bin\Qt5MultimediaWidgets.dll release\deps
cp $qt5_path\bin\Qt5Network.dll release\deps
cp $qt5_path\bin\Qt5Nfc.dll release\deps
cp $qt5_path\bin\Qt5OpenGL.dll release\deps
cp $qt5_path\bin\Qt5Positioning.dll release\deps
cp $qt5_path\bin\Qt5PrintSupport.dll release\deps
cp $qt5_path\bin\Qt5Widgets.dll release\deps
cp $qt5_path\bin\Qt5WebKit.dll release\deps
cp $qt5_path\bin\Qt5WebKitWidgets.dll release\deps
cp $qt5_path\bin\Qt5Sensors.dll release\deps
cp $qt5_path\bin\Qt5Quick.dll release\deps
cp $qt5_path\bin\Qt5Script.dll release\deps
cp $qt5_path\bin\Qt5Qml.dll release\deps
cp $qt5_path\bin\Qt5XmlPatterns.dll release\deps
cp $qt5_path\bin\Qt5WebChannel.dll release\deps
cp $qt5_path\bin\Qt5Sql.dll release\deps
cp $qt5_path\bin\Qt5Xml.dll release\deps
cp ..\huggle\Resources\huggle.ico huggle.ico
cp ..\huggle\Resources\huggle.ico release
cp $openssl_path\bin\ssleay32.dll release\deps
cp $openssl_path\bin\libeay32.dll release\deps
echo "Making package out of this"

$nsis_file = "Huggle.nsi"
if ($mingw)
{
    $nsis_file = "HuggleMinGW.nsi"
}

& $nsis_path $nsis_file

Read-Host -Prompt "Press Enter to continue"

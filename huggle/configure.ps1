# This is a powershell script that prepare huggle project so that it can be built

#  Copyright (c) 2013 - 2014, Petr Bena
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
    [bool]$pause = $true,
    [bool]$qtcreator = $true,
    [string]$compiler = "GCC"
)

$ErrorActionPreference = "Stop"

if (!(Test-Path "update.sh"))
{
    echo "This isn't a huggle build folder, you need to run this script from within the huggle folder"
    exit 1
}

echo "Creating version.txt"
if ((Test-Path "..\.git") -and (Get-Command git -errorAction SilentlyContinue))
{
    $revision_count = git rev-list HEAD --count
    $hash = git describe --always --tags
    echo "build: $revision_count $hash" | Set-Content version.txt
} else
{
    if (Test-Path("..\.git\FETCH_HEAD"))
    {
        $first, $lines = cat "..\.git\FETCH_HEAD"
        # Do not use the > trick per http://stackoverflow.com/questions/22406496/what-is-a-difference-between-operator-and-set-content-cmdlet
        echo $first | %{$_ -replace "[\t\s].*", ""} | Set-Content version.txt
    } else
    {
        echo "Development (non-git)" > version.txt
    }
}

if ($qtcreator -and !(Test-Path "huggle.pro"))
{
    echo "Creating huggle.pro"
    Copy-Item "huggle.orig" "huggle.pro"
}


$definitions_created = 0;

if (!(Test-Path "definitions.hpp"))
{
    echo "Preparing definitions file..."
    Copy-Item "definitions_prod.hpp" "definitions.hpp"
    $definitions_created = 1;
}

#echo "Checking if there is breakpad in your build folder"

#if (!(Test-Path("huggle.tmp")) -and !(Test-Path("exception_handler.lib")))
#{
#    echo "There is no breakpad installed on your system, disabling it"
#    Move-Item "huggle.pro" "huggle.tmp"
#    $replace = @"
#!!!!!!!  #This line was replaced with power shell, DO NOT COMMIT THIS CHANGE
#!!!!!!!  #LIBS +=  ..\huggle\exception_handler.lib ..\huggle\crash_generation_client.lib
#"@
#    cat "huggle.tmp" | %{$_ -replace ".*exception_handler\.lib.*", $replace} | Set-Content huggle.pro
#    if ($definitions_created -eq 1)
#    {
#        $replace = @"
#// This line was replaced with power shell, DO NOT COMMIT THIS CHANGE
##define DISABLE_BREAKPAD
#// End of modified code, nothing else changed
#"@
#        cat "definitions_prod.hpp" | %{$_ -replace ".*DISABLE_BREAKPAD.*", $replace} | Set-Content definitions.hpp
#    }
#}

echo "It's all done, you can build huggle now from within Qt creator!"
if ($pause)
{
    Read-Host -Prompt "Press Enter to continue"
}

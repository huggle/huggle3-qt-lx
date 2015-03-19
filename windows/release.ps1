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

$ErrorActionPreference = "Stop"

$root_path=$PWD

function Build-VisualStudioSolution            
{            
    param            
    (            
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [String] $SourceCodePath = "C:\SourceCode\Development\",            
            
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [String] $SolutionFile,            
                    
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [String] $Configuration = "Debug",            
                    
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [Boolean] $AutoLaunchBuildLog = $false,            
            
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [Switch] $MsBuildHelp,            
                    
        [parameter(Mandatory=$false)]            
        [ValidateNotNullOrEmpty()]             
        [Switch] $CleanFirst,            
                    
        [ValidateNotNullOrEmpty()]             
        [string] $BuildLogFile,            
               
	[ValidateNotNullOrEmpty()]                  
        [string] $BuildLogOutputPath = $env:userprofile + "\Desktop\"            
    )            
                
    process            
    {            
        # Local Variables            
        $MsBuild = $env:systemroot + "\Microsoft.NET\Framework\v2.0.50727\MSBuild.exe";            
                
        # Caller requested MsBuild Help?            
        if($MsBuildHelp)            
        {            
                $BuildArgs = @{            
                    FilePath = $MsBuild            
                    ArgumentList = "/help"            
                    Wait = $true            
                    RedirectStandardOutput = "C:\MsBuildHelp.txt"            
                }            
            
                # Get the help info and show            
                Start-Process @BuildArgs            
                Start-Process -verb Open "C:\MsBuildHelp.txt";            
        }            
        else            
        {            
            # Local Variables            
            $SlnFilePath = $SourceCodePath + $SolutionFile;            
            $SlnFileParts = $SolutionFile.Split("\");            
            $SlnFileName = $SlnFileParts[$SlnFileParts.Length - 1];            
            $BuildLog = $BuildLogOutputPath + $BuildLogFile            
            $bOk = $true;            
                        
            try            
            {            
                # Clear first?            
                if($CleanFirst)            
                {            
                    # Display Progress            
                    Write-Progress -Id 20275 -Activity $SlnFileName  -Status "Cleaning..." -PercentComplete 10;            
                            
                    $BuildArgs = @{            
                        FilePath = $MsBuild            
                        ArgumentList = $SlnFilePath, "/t:clean", ("/p:Configuration=" + $Configuration), "/v:minimal"            
                        RedirectStandardOutput = $BuildLog            
                        Wait = $true            
                        #WindowStyle = "Hidden"            
                    }            
            
                    # Start the build            
                    Start-Process @BuildArgs #| Out-String -stream -width 1024 > $DebugBuildLogFile             
                                
                    # Display Progress            
                    Write-Progress -Id 20275 -Activity $SlnFileName  -Status "Done cleaning." -PercentComplete 50;            
                }            
            
                # Display Progress            
                Write-Progress -Id 20275 -Activity $SlnFileName  -Status "Building..." -PercentComplete 60;            
                            
                # Prepare the Args for the actual build            
                $BuildArgs = @{            
                    FilePath = $MsBuild            
                    ArgumentList = $SlnFilePath, "/t:rebuild", ("/p:Configuration=" + $Configuration), "/v:minimal"            
                    RedirectStandardOutput = $BuildLog            
                    Wait = $true            
                    #WindowStyle = "Hidden"            
                }            
            
                # Start the build            
                Start-Process @BuildArgs #| Out-String -stream -width 1024 > $DebugBuildLogFile             
                            
                # Display Progress            
                Write-Progress -Id 20275 -Activity $SlnFileName  -Status "Done building." -PercentComplete 100;            
            }            
            catch            
            {            
                $bOk = $false;            
                Write-Error ("Unexpect error occured while building " + $SlnFileParts[$SlnFileParts.Length - 1] + ": " + $_.Message);            
            }            
                        
            # All good so far?            
            if($bOk)            
            {            
                #Show projects which where built in the solution            
                #Select-String -Path $BuildLog -Pattern "Done building project" -SimpleMatch            
                            
                # Show if build succeeded or failed...            
                $successes = Select-String -Path $BuildLog -Pattern "Build succeeded." -SimpleMatch            
                $failures = Select-String -Path $BuildLog -Pattern "Build failed." -SimpleMatch            
                            
                if($failures -ne $null)            
                {            
                    Write-Warning ($SlnFileName + ": A build failure occured. Please check the build log $BuildLog for details.");            
                }            
                            
                # Show the build log...            
                if($AutoLaunchBuildLog)            
                {            
                    Start-Process -verb "Open" $BuildLog;            
                }            
            }            
        }            
    }            
                
    <#
        .SYNOPSIS
        Executes the v2.0.50727\MSBuild.exe tool against the specified Visual Studio solution file.
        
        .Description
        
        .PARAMETER SourceCodePath
        The source code root directory. $SolutionFile can be relative to this directory. 
        
        .PARAMETER SolutionFile
        The relative path and filename of the Visual Studio solution file.
        
        .PARAMETER Configuration
        The project configuration to build within the solution file. Default is "Debug".
        
        .PARAMETER AutoLaunchBuildLog
        If true, the build log will be launched into the default viewer. Default is false.
        
        .PARAMETER MsBuildHelp
        If set, this function will run MsBuild requesting the help listing.
        
        .PARAMETER CleanFirst
        If set, this switch will cause the function to first run MsBuild as a "clean" operation, before executing the build.
        
        .PARAMETER BuildLogFile
        The name of the file which will contain the build log after the build completes.
        
        .PARAMETER BuildLogOutputPath
        The full path to the output folder where build log files will be placed. Defaults to the current user's desktop.
        
        .EXAMPLE
        
        .LINK
        http://stackoverflow.com/questions/2560652/why-does-powershell-fail-to-build-my-net-solutions-file-is-being-used-by-anot
        http://geekswithblogs.net/dwdii
        
        .NOTES
        Name:   Build-VisualStudioSolution
        Author: Daniel Dittenhafer
    #>                
}



if (!(Test-Path "gpl.txt"))
{
    echo "Unable to find license, are you in right folder?"
    exit 1
}

if (!(Test-Path "../huggle/configure.ps1"))
{
    echo "This isn't a huggle windows folder, you need to run this script from within the ROOT/windows folder"
    exit 1
}

if ((Test-Path ".\build"))
{
    echo "The build folder is already present, please remove it first"
    exit 1
}

echo "Configuring the project"

cd ..\huggle
.\configure.ps1 -pause $false -qtcreator $false

#let's try to invoke cmake now
cd $root_path
mkdir build
cd build
cmake ..\..\huggle\ -DCMAKE_PREFIX_PATH:STRING=C:\Qt\5.4\msvc2013\ -Wno-dev=true -DHUGGLE_EXT=true -DQT5_BUILD=true
Build-VisualStudioSolution -SourceCodePath="$PWD" -SolutionFile="huggle.sln" -Configuration="Release" -BuildLogOutputPath="$PWD"



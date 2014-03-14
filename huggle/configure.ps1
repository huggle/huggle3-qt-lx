# This is a powershell script that prepare huggle project so that it can be built

$ErrorActionPreference = "Stop"

if (!(Test-Path "update.sh"))
{
    echo "This isn't a huggle build folder, you need to run this script from within the huggle folder"
    exit 1
}

echo "Creating version.txt"
if (Test-Path("..\.git\FETCH_HEAD"))
{
    $first, $lines = cat "..\.git\FETCH_HEAD"
    echo $first | %{$_ -replace " .*", ""} | Set-Content version.txt
} else
{
    echo "Development (non-git)" > version.txt
}

if (!(Test-Path "definitions.hpp"))
{
    echo "Preparing definitions file..."
    Copy-Item "definitions_prod.hpp" "definitions.hpp"
}

echo "Checking if there is breakpad in your build folder"

if (!(Test-Path("huggle.tmp")) -and !(Test-Path("exception_handler.lib")))
{
    echo "There is no breakpad installed on your system, disabling it"
    Move-Item "huggle.pro" "huggle.tmp"
    $replace = @"
#!!!!!!!  #This line was replaced with power shell, DO NOT COMMIT THIS CHANGE
#!!!!!!!  #LIBS +=  ..\huggle\exception_handler.lib ..\huggle\crash_generation_client.lib
"@
    cat "huggle.tmp" | %{$_ -replace ".*exception_handler\.lib.*", $replace} | Set-Content huggle.pro
        $replace = @"
// This line was replaced with power shell, DO NOT COMMIT THIS CHANGE
#define DISABLE_BREAKPAD
// End of modified code, nothing else changed
"@
    cat "definitions_prod.hpp" | %{$_ -replace ".*DISABLE_BREAKPAD.*", $replace} | Set-Content definitions.hpp
}

echo "It's all done, you can build huggle now from withing Qt creator!"
pause
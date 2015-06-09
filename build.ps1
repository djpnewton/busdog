param
(
    [Parameter(Position=1, Mandatory=$True)]
    [ValidateSet("Win32", "x64")]
    [string] $platform,

    [Parameter(Position=2, Mandatory=$True)]
    [ValidateSet("Release", "Debug")]
    [string] $configuration
)

function Exec
{
    [CmdletBinding()]
    param (
        [Parameter(Position=0, Mandatory=1)]
        [scriptblock]$Command,
        [Parameter(Position=1, Mandatory=0)]
        [string]$ErrorMessage = "Execution of command failed.`n$Command"
    )
    & $Command
    if ($LastExitCode -ne 0) {
        throw "Exec: $ErrorMessage"
    }
}

# derive driver project configuration
if ($configuration -match "Release") {
    $conf_driver = "Win7 Release"
}
else {
    $conf_driver = "Win7 Debug"
}
# derive driver project platform dirs
if ($platform -match "Win32") {
    $plat_driver_output = "x86"
    $plat_driver_package_output = ""
}
else {
    $plat_driver_output = "x64"
    $plat_driver_package_output = "x64"
}
# replace spaces in configuration as for some reason MSVC likes to have a "Win7 Release" configuration and then output files at "./Win7Release"
$conf_nosp = $conf_driver -replace '\s',''

# set environment variables for Visual Studio Command Prompt
pushd 'c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC'
cmd /c "vcvarsall.bat&set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd
write-host "`nVisual Studio 2010 Command Prompt variables set." -ForegroundColor Yellow

$ErrorActionPreference = "Stop"

write-host "** building busdog.sys" -ForegroundColor Yellow
Exec { msbuild ./filter/busdog.sln /t:clean }
Exec { msbuild ./filter/busdog.sln /p:Configuration="$conf_driver" /property:Platform="$platform" }

write-host "** built busdog.sys" -ForegroundColor Yellow
write-host "   sign filter/$conf_nosp/busdog-Package/busdog.sys and press any key to continue" -ForegroundColor Yellow
$x = $host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

write-host "** copy driver resources to gui\driverRes\bin" -ForegroundColor Yellow
rm -Recurse -Force gui\driverRes\bin\*
write-host "   wait for file deletion (WTF powershell?!?)" -ForegroundColor Yellow
Start-Sleep -s 2
copy -force "filter/$conf_nosp/$plat_driver_output/busdog.pdb" gui\driverRes\bin
copy -force "filter/$plat_driver_package_output/$conf_nosp/busdog-Package/busdog.sys" gui\driverRes\bin
copy -force "filter/$plat_driver_package_output/$conf_nosp/busdog-Package/busdog.inf" gui\driverRes\bin
copy -force "filter/$plat_driver_package_output/$conf_nosp/busdog-Package/wdfcoinstaller?????.dll" gui\driverRes\bin
copy -force "$env:WindowsSdkDir/Redist/DIFx/dpinst/MultiLin/$plat_driver_output/dpinst.exe" gui\driverRes\bin

write-host "** building busdog.exe" -ForegroundColor Yellow
Exec { msbuild .\gui\build.xml /t:clean }
Exec { msbuild .\gui\build.xml /p:Configuration="$configuration" }

write-host "** built gui/busdog/bin/busdog.exe" -ForegroundColor Yellow
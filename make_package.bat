:: ddk environment
set DDKBUILDENV=
call C:\WinDDK\7600.16385.0\bin\setenv.bat C:\WinDDK\7600.16385.0\ fre x86 WXP no_oacr 
cd %~dp0

:: build busdog filter driver
build -ceZg
if exist buildfre_wxp_x86.err goto error
echo +++++++++++++++++++++++++++++++++
echo +++busdog filter driver built!+++
echo +++++++++++++++++++++++++++++++++

:: copy driver to busdog gui directory
xcopy /Y filter\objfre_wxp_x86\i386\busdog.sys gui\driverRes\bin
if errorlevel 1 goto error

::visual studio environment
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
cd %~dp0

:: build busdog gui
cd gui
msbuild build.xml /t:clean
if errorlevel 1 goto error
msbuild build.xml /t:release
if errorlevel 1 goto error
echo +++++++++++++++++++++++
echo +++busdog gui built!+++
echo +++++++++++++++++++++++


:: finito!
goto end

:error
echo +++++++++++++++++++++
echo +++Error in build!+++
echo +++++++++++++++++++++

:end
cd %~dp0



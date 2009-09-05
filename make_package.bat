:: ddk environment
if "%1"=="64" (
    call C:\WinDDK\7600.16385.0\bin\setenv.bat C:\WinDDK\7600.16385.0\ fre x64 WNET no_oacr 
) else (
    call C:\WinDDK\7600.16385.0\bin\setenv.bat C:\WinDDK\7600.16385.0\ fre x86 WXP no_oacr 
)
if "%1"=="64" (
    set other_arch_spec=amd64
) else (
    set other_arch_spec=i386
)
cd %~dp0

:: build busdog filter driver
build -ceZg
if exist buildfre_%_buildtype%_%_BUILDARCH%.err goto error
echo +++++++++++++++++++++++++++++++++
echo +++busdog filter driver built!+++
echo +++++++++++++++++++++++++++++++++

:: copy driver to busdog gui directory
xcopy /Y filter\obj%BUILD_ALT_DIR%\%other_arch_spec%\busdog.sys gui\driverRes\bin
if errorlevel 1 goto error
xcopy /Y filter\obj%BUILD_ALT_DIR%\%other_arch_spec%\busdog.inf gui\driverRes\bin
if errorlevel 1 goto error
xcopy /Y %BASEDIR%\redist\wdf\%_BUILDARCH%\wdfcoinstaller?????.dll gui\driverRes\bin
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
exit /B 1

:end
cd %~dp0
exit /B 0



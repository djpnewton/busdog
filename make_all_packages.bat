del busdog32.exe
del busdog64.exe

:: 32 bit release
setlocal
call make_package.bat 32
endlocal
if errorlevel 1 goto error
echo f | xcopy /Y gui\release\busdog.exe busdog32.exe
if errorlevel 1 goto error

:: 64 bit release
setlocal
call make_package.bat 64
endlocal
if errorlevel 1 goto error
echo f | xcopy /Y gui\release\busdog.exe busdog64.exe
if errorlevel 1 goto error

:: finito!
goto end

:error
echo +++++++++++++++++++++++++++++++++
echo +++Error in make_all_packages!+++
echo +++++++++++++++++++++++++++++++++
exit /B 1

:end
exit /B 0

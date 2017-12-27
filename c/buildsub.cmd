@echo off
SETLOCAL
cd %1
cd
wmake %2 %3 %4 %5 %6 -h -e
if errorlevel 1 goto error
goto end

:error
echo.
echo Notice any WMAKE errors displayed, press a key to edit logs, if any ...
pause > nul
e *.err

:end
ENDLOCAL

@echo off
rem Put your favourite target(s) first, it will be built by default

if "%1" == "?" goto help
if "%1" == "min" goto min

call buildsub.cmd _txall_\os2d %2 %3 %4 %5

REM end of default built targets
if "%1" == ""    goto end
if "%1" == "def" goto end

call buildsub.cmd _txall_\dosd %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\dosr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\dost %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\lind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\lint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\os2r %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\os2t %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\wind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\winr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\wint %2 %3 %4 %5
if errorlevel 1 goto end

call buildsub.cmd _txmin_\dosr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\winr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\os2r %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\dost %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\wint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\lint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\os2t %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\dosd %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\wind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\lind %2 %3 %4 %5
if errorlevel 1 goto end

:min
call buildsub.cmd _txmin_\os2d %2 %3 %4 %5
if errorlevel 1 goto end
goto end

:help
echo.
echo Build one or more executable targets using a small 'makefile' in a 2nd
echo level subdirectory for each target-type and a generic makefile.mif here
echo.
echo Usage:  %0  [def ^| all ^| min]  [clean]
echo.
echo Parameters:  def     Build trace (but not debug) for all platforms
echo              all     Build all target-types, trace and debug
echo              min     Build default MINimal-size target
echo.
echo              clean   Cleanup, delete all previously built stuff and temporaries
echo.
:end

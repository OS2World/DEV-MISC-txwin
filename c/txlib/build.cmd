@echo off
REM updated for 15.0 Removed the TRACE versions (just debug or release)
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
call buildsub.cmd _txall_\lind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\os2r %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\wind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txall_\winr %2 %3 %4 %5
if errorlevel 1 goto end

call buildsub.cmd _txmin_\dosr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\winr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _txmin_\os2r %2 %3 %4 %5
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
echo Parameters:  def     Build default: _txall_ DEBUG version for OS/2
echo              all     Build all target-types, retail and debug
echo              min     Build default: _txmin_ DEBUG version for OS/2
echo.
echo              clean   Cleanup, delete all previously built stuff and temporaries
echo.
:end

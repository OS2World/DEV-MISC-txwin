@echo off
rem Put your favourite target(s) first, it will be built by default

if "%1" == "?" goto help

call buildsub.cmd _build_\os2t %2 %3 %4 %5
if errorlevel 1 goto end
copy /b _build_\*os2t.exe sample.exe

REM end of default built targets
if "%1" == ""    goto end
if "%1" == "def" goto end

call buildsub.cmd _build_\dosd %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\dosr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\dost %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\lind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\lint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\wind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\winr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\wint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\os2d %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd _build_\os2r %2 %3 %4 %5
if errorlevel 1 goto end
goto end

:help
echo.
echo Build one or more executable targets using a small 'makefile' in a
echo subdirectory for each target-type and a generic makefile.mif here
echo.
echo Usage:  %0  [def ^| all]  [clean]
echo.
echo Parameters:  def     Build trace (but not debug) for all platforms
echo              all     Build all target-types, trace and debug
echo.
echo              clean   Cleanup, delete all previously built stuff and temporaries
echo.
:end

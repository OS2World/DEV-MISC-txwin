@echo off
rem Put your favourite target(s) first, it will be built by default

if "%1" == "?" goto help

call buildsub.cmd build\os2d %2 %3 %4 %5
if errorlevel 1 goto end
copy build\txtos2d.exe txt.exe

REM end of default built targets
if "%1" == ""    goto end
if "%1" == "def" goto end

call buildsub.cmd build\dosd %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\dosr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\dost %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\lind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\linr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\lint %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\os2t %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\os2r %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\wind %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\winr %2 %3 %4 %5
if errorlevel 1 goto end
call buildsub.cmd build\wint %2 %3 %4 %5
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

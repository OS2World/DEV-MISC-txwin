@echo off
if "%2" == "clean" goto clean
@build %1 %2 %3 %4 %5 %6 %7 %8 | tee b.log
goto end
:clean
@build %1 %2 %3 %4 %5 %6 %7 %8
:end

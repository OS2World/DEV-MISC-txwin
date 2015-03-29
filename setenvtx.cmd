@echo off
rem
rem Update the driveletter/install-path to match your installation (H: ==> x:)
rem or call the script with driveletter (and optional path) as parameters
rem
rem Make sure you also add the TXPATH directory to the DOS PATH in AUTOEXEC.BAT
rem to allow the makefiles to find the UPX compression utility
rem
rem You may have to set or update the WATCOM environment variable too.
rem
echo TxWin develop environment
echo Use toolkits from OpenWatcom unless a toolkit environment variable exists

set txdrive=H:
if "%1"=="" goto driveset
set txdrive=%1
:driveset

set txpath=\c
if "%2"=="" goto pathset
set txpath=%2
:pathset

if "%screenwidth%"=="768" goto smallscreen
mode 102,38
goto screendone
:smallscreen
mode 82,28
:screendone

%txdrive%
cd %txpath%\txlib
SET PROMPT=RC:$e[0;1;33m$r $e[0;1;36m[TxWin development] $p -$g $e[0m
if not "%watcom%"=="" goto watcomset
rem Use to compile with the development REL2 compiler (includes Linux targets)
set watcom=%txdrive%\ow\rel2
:watcomset

:rest
set path=%watcom%\binp;%watcom%\binw;%txdrive%%txpath%;%path%
set beginlibpath=%watcom%\binp\dll
set edpath=%watcom%\eddat
set help=%watcom%\binp\help;%help%
set bookshelf=%watcom%\binp\help;%bookshelf%
set include=%watcom%\h;%txdrive%%txpath%\txlib\include;..\..;..;.
if "%toolkit%"=="" goto internal
set os2_include=%toolkit%\h
set os2tklib=%toolkit%\lib
set dpath=%dpath%;%toolkit%\msg
goto os2tkset
:internal
set os2_include=%watcom%\h\os2
set os2tklib=%watcom%\lib386\os2
:os2tkset
set nt_include=%watcom%\h\nt
set wintklib=%watcom%\lib386\nt
set linux_include=%watcom%\lh
set lintklib=%watcom%\lib386\linux
set lib=

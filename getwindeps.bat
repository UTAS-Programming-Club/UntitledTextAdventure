:: Requires windows 10 or 11 for built in curl
@echo off

set COSMOS=third_party\cosmos\bin

if not exist %COSMOS% mkdir %COSMOS%

:: For Windows
if not exist %COSMOS%\make.exe curl.exe -o %COSMOS%\make.exe https://cosmo.zip/pub/cosmos/bin/make
if not exist %COSMOS%\unzip.exe curl.exe -o %COSMOS%\unzip.exe https://cosmo.zip/pub/cosmos/bin/unzip

:: For Make and compiler
if not exist %COSMOS%\cat curl.exe -o %COSMOS%\cat https://cosmo.zip/pub/cosmos/bin/cat
if not exist %COSMOS%\cp curl.exe -o %COSMOS%\cp https://cosmo.zip/pub/cosmos/bin/cp.ape
if not exist %COSMOS%\mkdir curl.exe -o %COSMOS%\mkdir https://cosmo.zip/pub/cosmos/bin/mkdir.ape
if not exist %COSMOS%\mv curl.exe -o %COSMOS%\mv https://cosmo.zip/pub/cosmos/bin/mv.ape
if not exist %COSMOS%\rm curl.exe -o %COSMOS%\rm https://cosmo.zip/pub/cosmos/bin/rm.ape
if not exist %COSMOS%\sed curl.exe -o %COSMOS%\sed https://cosmo.zip/pub/cosmos/bin/sed
if not exist %COSMOS%\sh curl.exe -o %COSMOS%\sh https://cosmo.zip/pub/cosmos/bin/dash
if not exist %COSMOS%\tr curl.exe -o %COSMOS%\tr https://cosmo.zip/pub/cosmos/bin/tr

:: if not exist %COSMOS%\echo curl.exe -o %COSMOS%\echo https://cosmo.zip/pub/cosmos/bin/echo.ape
:: if not exist %COSMOS%\ls curl.exe -o %COSMOS%\ls https://cosmo.zip/pub/cosmos/bin/ls
:: if not exist %COSMOS%\pwd curl.exe -o %COSMOS%\pwd https://cosmo.zip/pub/cosmos/bin/pwd

if not exist %COSMOS%\x86_64-unknown-cosmo-cc (
  curl.exe -o %COSMOS%\cosmocc.zip https://cosmo.zip/pub/cosmocc/cosmocc-3.3.3.zip
  REM From https://stackoverflow.com/a/52517718
  REM Run unzip as admin which is required to create symlinks
  cd /d "%~dp0" && ( if exist "%temp%\getadmin.vbs" del "%temp%\getadmin.vbs" ) && fsutil dirty query %systemdrive% 1>nul 2>nul || (  echo Set UAC = CreateObject^("Shell.Application"^) : UAC.ShellExecute "cmd.exe", "/c cd ""%~sdp0"" && %COSMOS%\unzip.exe %COSMOS%\cosmocc.zip -d %COSMOS%\..\ && del %COSMOS%\cosmocc.zip", "", "runas", 1 >> "%temp%\getadmin.vbs" && "%temp%\getadmin.vbs" )

  :WaitForUnzip
  if not exist %COSMOS%\cosmocc.zip goto UnzipDone
  timeout /t 1 >nul
  goto WaitForUnzip

  :UnzipDone
  echo >nul
)

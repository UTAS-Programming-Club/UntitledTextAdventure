:: Requires windows 10 or 11 for built in curl
@echo off

set COSMOS=%0\..\third_party\cosmos\bin
set COSMOS_URL=https://cosmo.zip/pub/cosmos/bin

if %1.==-f. del /f /q /s %COSMOS%\.. >nul 2>nul

if not exist %COSMOS% mkdir %COSMOS%

:: For Windows
if not exist %COSMOS%\unzip.exe curl.exe -o %COSMOS%\unzip.exe %COSMOS_URL%/unzip

:: For Make and compiler
if not exist %COSMOS%\cat curl.exe -o %COSMOS%\cat %COSMOS_URL%/cat
if not exist %COSMOS%\cp curl.exe -o %COSMOS%\cp %COSMOS_URL%/cp.ape
if not exist %COSMOS%\cut curl.exe -o %COSMOS%\cut %COSMOS_URL%/cut
if not exist %COSMOS%\grep curl.exe -o %COSMOS%\grep %COSMOS_URL%/grep
if not exist %COSMOS%\ls curl.exe -o %COSMOS%\ls %COSMOS_URL%/ls
if not exist %COSMOS%\md5sum curl.exe -o %COSMOS%\md5sum %COSMOS_URL%/md5sum
if not exist %COSMOS%\mkdir curl.exe -o %COSMOS%\mkdir %COSMOS_URL%/mkdir.ape
if not exist %COSMOS%\mv curl.exe -o %COSMOS%\mv %COSMOS_URL%/mv.ape
if not exist %COSMOS%\rm curl.exe -o %COSMOS%\rm %COSMOS_URL%/rm.ape
if not exist %COSMOS%\sed curl.exe -o %COSMOS%\sed %COSMOS_URL%/sed
if not exist %COSMOS%\sh curl.exe -o %COSMOS%\sh %COSMOS_URL%/dash
if not exist %COSMOS%\tr curl.exe -o %COSMOS%\tr %COSMOS_URL%/tr
if not exist %COSMOS%\uname curl.exe -o %COSMOS%\uname %COSMOS_URL%/uname.ape

if not exist %COSMOS%\x86_64-unknown-cosmo-cc (
  curl.exe -o %COSMOS%\cosmocc.zip https://cosmo.zip/pub/cosmocc/cosmocc-3.5.4.zip
  REM From https://stackoverflow.com/a/52517718
  REM Run unzip as admin which is required to create symlinks

  net session >nul 2>&1
  if %errorLevel% == 0 (
    %COSMOS%\unzip.exe %COSMOS%\cosmocc.zip -d %COSMOS%\..\
    del %COSMOS%\cosmocc.zip
  ) else (
    cd /d "%~dp0" && ( if exist "%temp%\getadmin.vbs" del "%temp%\getadmin.vbs" ) && fsutil dirty query %systemdrive% 1>nul 2>nul || (  echo Set UAC = CreateObject^("Shell.Application"^) : UAC.ShellExecute "cmd.exe", "/c cd ""%~sdp0"" && %COSMOS%\unzip.exe %COSMOS%\cosmocc.zip -d %COSMOS%\..\ && del %COSMOS%\cosmocc.zip", "", "runas", 1 >> "%temp%\getadmin.vbs" && "%temp%\getadmin.vbs" )
  )

  :WaitForUnzip
  if not exist %COSMOS%\cosmocc.zip goto UnzipDone
  timeout /t 1 >nul
  goto WaitForUnzip

  :UnzipDone
  echo >nul
)

if not exist %COSMOS%\apelink.exe copy %COSMOS%\apelink %COSMOS%\apelink.exe
if not exist %COSMOS%\echo copy %COSMOS%\echo.ape %COSMOS%\echo
if not exist %COSMOS%\make.exe copy %COSMOS%\make %COSMOS%\make.exe

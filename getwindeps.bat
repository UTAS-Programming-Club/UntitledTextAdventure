:: Requires windows 10 or 11 for built in curl
@echo off
setlocal enableDelayedExpansion

set COSMOS=%~dp0third_party\cosmos\bin
set COSMOS_URL=https://cosmo.zip/pub/cosmos/v/3.7.1/bin/

if %1.==-f. del /f /q /s %COSMOS%\.. >nul 2>nul

if not exist %COSMOS% mkdir %COSMOS%

if not exist %COSMOS%\cat curl -o %COSMOS%\cat %COSMOS_URL%/cat
if not exist %COSMOS%\cut curl -o %COSMOS%\cut %COSMOS_URL%/cut
if not exist %COSMOS%\grep curl -o %COSMOS%\grep %COSMOS_URL%/grep
if not exist %COSMOS%\ls curl -o %COSMOS%\ls %COSMOS_URL%/ls
if not exist %COSMOS%\md5sum curl -o %COSMOS%\md5sum %COSMOS_URL%/md5sum
if not exist %COSMOS%\sed curl -o %COSMOS%\sed %COSMOS_URL%/sed
if not exist %COSMOS%\uname curl -o %COSMOS%\uname %COSMOS_URL%/uname.ape

if not exist %COSMOS%\x86_64-unknown-cosmo-cc (
  curl -o %COSMOS%\cosmocc.zip https://cosmo.zip/pub/cosmocc/cosmocc-3.7.1.zip
  tar -xvf %COSMOS%\cosmocc.zip -C %COSMOS%\..\
  del %COSMOS%\cosmocc.zip
)

if not exist %COSMOS%\cp copy %COSMOS%\cp.ape %COSMOS%\cp
if not exist %COSMOS%\mkdir copy %COSMOS%\mkdir.ape %COSMOS%\mkdir
if not exist %COSMOS%\rm copy %COSMOS%\rm.ape %COSMOS%\rm
if not exist %~dp0\build\bootstrap\sh.exe copy %~dp0\build\bootstrap\sh %~dp0\build\bootstrap\sh.exe

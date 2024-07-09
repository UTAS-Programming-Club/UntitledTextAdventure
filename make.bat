@echo off

set COSMOSBIN=%cd:\=/%/third_party/cosmos/bin
set SHELL=sh

%COSMOSBIN%/make.exe %* ^
PATH=%COSMOSBIN% ^
APELINK="apelink.exe" x86_64APEELF="%COSMOSBIN%/ape-x86_64.elf" ^
AR="x86_64-unknown-cosmo-ar" ^
AS="x86_64-unknown-cosmo-as" ^
CC="%SHELL% %COSMOSBIN%/x86_64-unknown-cosmo-cc" ^
CXX="%SHELL% %COSMOSBIN%/x86_64-unknown-cosmo-c++" ^
MAKE=make ^
SHELL=%SHELL%

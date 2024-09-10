@echo off

setlocal
set COSMOSBIN=%~dp0third_party\cosmos\bin
set POSIXCOSMOBIN=%COSMOSBIN:\=/%
set PATH=%~dp0/build/bootstrap;%COSMOSBIN%;%PATH%

set APELINK=apelink -l %POSIXCOSMOBIN%/ape-x86_64.elf
set AR=%POSIXCOSMOBIN%/ar.ape
set AS=sh %POSIXCOSMOBIN%/x86_64-unknown-cosmo-as
set CC=sh %POSIXCOSMOBIN%/x86_64-unknown-cosmo-cc
set CXX=sh %POSIXCOSMOBIN%/x86_64-unknown-cosmo-c++
sh build/batchtodash.sh make SHELL=sh %*
endlocal

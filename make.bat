@echo off

set COSMOS=third_party\cosmos\bin
set MAKECOSMOS=third_party/cosmos/bin
set SUBMAKECOSMOS=../cosmos/bin
set SHELL=sh
set CC=x86_64-unknown-cosmo-cc

%COSMOS%\make %* SHELL=%MAKECOSMOS%/%SHELL% PATH=%MAKECOSMOS% SUBMAKESHELL=SHELL=%SUBMAKECOSMOS%/%SHELL% SUBMAKEPATH=PATH=%SUBMAKECOSMOS% CC="%MAKECOSMOS%/%SHELL% %MAKECOSMOS%/%CC%" SUBMAKECC=CC="\"%SUBMAKECOSMOS%/%SHELL% %SUBMAKECOSMOS%/%CC%\""
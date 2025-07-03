@ECHO OFF
SETLOCAL

:: TODO: Support using an existing copy of haxe
:: TODO: Support aarch64?
:: TODO: Update to Haxe 4.3.7
:: TODO: Update to Neko 2.4.1
:: TODO: Update to hxcpp 4.3.90

SET "haxe_version=4.3.6"
SET "neko_version=2.4.0"
SET "neko_version_2=2-4-0"
SET "hxlib_version=4.3.2"
SET "hxlib_version_2=4,3,2"

SET "base_dir=%CD%\build\haxe"
SET "haxe_dir=%base_dir%\haxe"
SET "neko_dir=%base_dir%\neko"
SET "haxelib_dir=%base_dir%\.haxelib"

SET "need_haxe=false"
SET "need_neko=false"
SET "need_subshell=false"

GOTO :after_functions

:: Parameters:
:: 1. Informative name
:: 2. url
:: 3. destination path
:download_file
ECHO Downloading %~1
CALL curl -Lf "%~2" -o "%~3"
GOTO :EOF

:: Parameters:
:: 1. Informative name
:: 2. archive path
:: 3. destination path
:extract_zip
ECHO Extracting %~1
MKDIR "%~3"
CALL tar -xf "%~2" -C "%~3" --strip-components=1
GOTO :EOF

:: From https://stackoverflow.com/a/53464542
:GETPARENT
SET "PSCMD=$ppid=$pid;while($i++ -lt 3 -and ($ppid=(Get-CimInstance Win32_Process -Filter ('ProcessID='+$ppid)).ParentProcessId)) {}; (Get-Process -EA Ignore -ID $ppid).Name"
FOR /f "tokens=*" %%i IN ('powershell -noprofile -command "%PSCMD%"') DO SET %1=%%i
GOTO :EOF


:after_functions
IF "%*"=="-f" (
  :: From https://stackoverflow.com/a/32607287
  DEL /f /s /q "%haxelib_dir%" "%haxe_dir%" "%neko_dir%" out > NUL 2>&1
  RMDIR /s /q "%haxelib_dir%" "%haxe_dir%" "%neko_dir%" out > NUL 2>&1
)

IF NOT EXIST "%haxe_dir%\" (
  CALL :download_file Haxe https://github.com/HaxeFoundation/haxe/releases/download/%haxe_version%/haxe-%haxe_version%-win64.zip haxe.zip
  CALL :extract_zip Haxe haxe.zip "%haxe_dir%"
  DEL haxe.zip
  ECHO:
  SET "need_haxe=true"
)

IF NOT EXIST "%neko_dir%\" (
  CALL :download_file Neko https://github.com/HaxeFoundation/neko/releases/download/v%neko_version_2%/neko-%neko_version%-win64.zip neko.zip
  CALL :extract_zip Neko neko.zip "%neko_dir%"
  DEL neko.zip
  ECHO:
  SET "need_neko=true"
)

IF NOT EXIST "%haxelib_dir%\" (
  ECHO Creating haxelib repo
  SETLOCAL
  SET "PATH=%neko_dir%;%PATH%"
  CALL "%haxe_dir%/haxelib" setup "%haxelib_dir%"
  ENDLOCAL
  ECHO:
)

IF NOT EXIST "%haxelib_dir%\hxcpp\%hxlib_version_2%\" (
  ECHO Downloading hxcpp
  SETLOCAL
  SET "PATH=%neko_dir%;%PATH%"
  CALL "%haxe_dir%/haxelib" install hxcpp %hxlib_version%
  ENDLOCAL
  ECHO:
)

WHERE haxe > NUL 2>&1
IF %ERRORLEVEL% NEQ 0 SET "need_haxe=true"
IF "%need_haxe%"=="true" (
  SET "PATH=%haxe_dir%;%PATH%"
  SET "need_subshell=true"
)

WHERE neko > NUL 2>&1
IF %ERRORLEVEL% NEQ 0 SET "need_neko=true"
IF "%need_neko%"=="true" (
  SET "PATH=%neko_dir%;%PATH%"
  SET "need_subshell=true"
)

CALL :GETPARENT SHELL
IF "%need_subshell%"=="true" (
  ECHO Launching subshell
  %SHELL%
)

ENDLOCAL

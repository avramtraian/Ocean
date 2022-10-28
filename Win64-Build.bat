@ECHO OFF
PUSHD "%~dp0"

IF NOT EXIST "Bin\" ( MKDIR "Bin" )

SET CompilerFlags=-g
SET IncludeFlags=
SET LinkerFlags=-luser32.lib
SET Defines=

@ECHO Compiling...
clang Ocean.c %CompilerFlags% %IncludeFlags% %LinkerFlags% %Defines% -o "Bin/Ocean.exe"

IF %ERRORLEVEL% NEQ 0 ( goto :OnError )

@ECHO Done.
goto :EOF

:OnError
	@ECHO Compilation failed.
	PAUSE
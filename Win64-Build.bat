@ECHO OFF
PUSHD "%~dp0"

IF NOT EXIST "Bin\" ( MKDIR "Bin" )

SET CompilerFlags=-g -std=c++17
SET IncludeFlags=
SET LinkerFlags=-luser32.lib -lopengl32.lib -lgdi32.lib
SET Defines=-D_DEBUG

@ECHO Compiling...
clang++ Ocean.cpp %CompilerFlags% %IncludeFlags% %LinkerFlags% %Defines% -o "Bin/Ocean.exe"

IF %ERRORLEVEL% NEQ 0 ( goto :OnError )

@ECHO Done.
goto :EOF

:OnError
	@ECHO Compilation failed.
	PAUSE
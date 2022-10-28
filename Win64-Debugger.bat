@ECHO OFF
PUSHD "%~dp0"

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
devenv Bin\Ocean.exe
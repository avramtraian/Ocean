@echo off

if not exist "build/" ( mkdir "build" )
pushd "build"

set CompilerFlags=-Oi -Od -Zi -nologo /FC -DOCEAN_WINDOWS=1 -DOCEAN_COMPILER_MSVC=1 -DOCEAN_DEBUG=1
set LinkerFlags=-DEBUG -nologo -subsystem:windows user32.lib gdi32.lib

echo Compiling Source...
cl /c "../source/win32_ocean.cpp" %CompilerFlags%

echo Linking...
link "win32_ocean.obj" %LinkerFlags% /OUT:Ocean.exe

popd

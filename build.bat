:: Copyright (c) 2026 Traian Avram. All rights reserved.
:: SPDX-License-Identifier: BSD-3-Clause.

::==============================================================================
::----------------------- Configure execution environment ---------------------- 
::==============================================================================

@echo off
pushd "%~dp0"
if not exist "build/" ( mkdir build )
pushd "build"

::==============================================================================
::------------------------------- Set build flags ------------------------------ 
::==============================================================================

set compile_flags_common=  /nologo /EHs- /EHc-
set compile_flags_debug=   /Od /DBUILD_DEBUG=1 /Zi
set compile_flags_release= /O2 /DBUILD_RELEASE=1

set link_flags_common=     /nologo /INCREMENTAL:NO
set link_flags_debug=      /DEBUG:FULL
set link_flags_release=

set include_dirs=          /I"../src"
set link_libraries=        user32.lib gdi32.lib

::==============================================================================
::------------------------- Invoke compiler and linker ------------------------- 
::==============================================================================

set target_name=editor5

cl /c %compile_flags_common% %compile_flags_debug% %include_dirs% "../src/main.cpp" /Fo:"%target_name%.obj"
if %errorlevel% neq 0 ( goto build_failed )

link %link_flags_common% %link_flags_debug% "%target_name%.obj" %link_libraries% /OUT:"%target_name%.exe"
if %errorlevel% neq 0 ( goto build_failed )

::==============================================================================
::-------------------------- Exit from script execution ------------------------ 
::==============================================================================

:build_succeded
echo Build finished.
set build_error_code=0
goto exit_build

:build_failed
echo Build failed.
set build_error_code=1
goto exit_build

:exit_build
popd
popd
exit /b %build_error_code%

::============================================================::
:: Copyright (c) 2025-2026 Traian Avram. All rights reserved. ::
:: SPDX-License-Identifier: MIT.                              ::
::============================================================::

@echo off
pushd "%~dp0"
pushd "build"

cl "../src/os_windows.cpp" /Od /Zi /std:c++17 /nologo /I ../src/ /MD /link /OUT:editor3.exe

popd
popd

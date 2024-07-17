#
# Copyright (c) 2024 Traian Avram. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause.
#

#==========================================================================================
# PROJECT CONFIGURATION.
#==========================================================================================

cmake_minimum_required(VERSION 3.14)
project(Ocean CXX C)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Build/${CMAKE_BUILD_TYPE}")

# Make the CMake predefined targets appear nicer if using the Visual Studio toolchain.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_EXPORT_LIBRARY_PREFIX "")

#==========================================================================================
# EXECUTABLE CONFIGURATION.
#==========================================================================================

set(OCEAN_CORE_SOURCE_FILES
    Core/Assertion.h
    Core/Defines.h
    Core/EntryPoint.cpp
    Core/Forward.h
    Core/Math/Color.cpp
    Core/Math/Color.h
    Core/Math/Vector.h
    Core/Memory/Arena.cpp
    Core/Memory/Arena.h
    Core/Memory/MemoryOperations.cpp
    Core/Memory/MemoryOperations.h
    Core/Types.h
)

set(OCEAN_DRAW_SOURCE_FILES
    Draw/Bitmap.cpp
    Draw/Bitmap.h
    Draw/Forward.h
)

set(OCEAN_EDITOR_SOURCE_FILES
)

set(OCEAN_PLATFORM_SOURCE_FILES
    Platform/Memory.h
    Platform/OffscreenBitmap.h
    Platform/Window.h
    Platform/Windows/WindowsEntryPoint.h
    Platform/Windows/WindowsHeaders.h
    Platform/Windows/WindowsMemory.cpp
    Platform/Windows/WindowsOffscreenBitmap.cpp
    Platform/Windows/WindowsWindow.cpp
)

add_executable(Ocean WIN32
    ${OCEAN_CORE_SOURCE_FILES}
    ${OCEAN_DRAW_SOURCE_FILES}
    ${OCEAN_EDITOR_SOURCE_FILES}
    ${OCEAN_PLATFORM_SOURCE_FILES}
)

target_include_directories(Ocean PRIVATE "${CMAKE_SOURCE_DIR}")

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
    Core/Log.cpp
    Core/Log.h
    Core/Math/Color.cpp
    Core/Math/Color.h
    Core/Math/MathUtils.h
    Core/Math/Rect.h
    Core/Math/Vector.h
    Core/Memory/Arena.cpp
    Core/Memory/Arena.h
    Core/Memory/MemorySpan.h
    Core/Memory/MemoryOperations.cpp
    Core/Memory/MemoryOperations.h
    Core/Types.h
)

set(OCEAN_DRAW_SOURCE_FILES
    Draw/Bitmap.h
    Draw/Brush.h
    Draw/Software/SoftwareBitmap.cpp
    Draw/Software/SoftwareBrush.cpp
)

set(OCEAN_EDITOR_SOURCE_FILES
    Editor/Settings.cpp
    Editor/Settings.h
    Editor/State.h
)

set(OCEAN_PLATFORM_SOURCE_FILES
    Platform/FileSystem.h
    Platform/Memory.h
    Platform/OffscreenBitmap.h
    Platform/Window.h
    Platform/Windows/WindowsEntryPoint.h
    Platform/Windows/WindowsFileSystem.cpp
    Platform/Windows/WindowsHeaders.h
    Platform/Windows/WindowsMemory.cpp
    Platform/Windows/WindowsSoftwareOffscreenBitmap.cpp
    Platform/Windows/WindowsWindow.cpp
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(OCEAN_EXECUTABLE_TYPE "")
    set(OCEAN_CONFIGURATION_MACRO "OCEAN_CONFIGURATION_DEBUG=1")
else ()
    set(OCEAN_EXECUTABLE_TYPE WIN32)
    set(OCEAN_CONFIGURATION_MACRO "OCEAN_CONFIGURATION_RELEASE=1")
endif ()

add_executable(Ocean ${OCEAN_EXECUTABLE_TYPE}
    ${OCEAN_CORE_SOURCE_FILES}
    ${OCEAN_DRAW_SOURCE_FILES}
    ${OCEAN_EDITOR_SOURCE_FILES}
    ${OCEAN_PLATFORM_SOURCE_FILES}
)

target_compile_definitions(Ocean PRIVATE ${OCEAN_CONFIGURATION_MACRO})
target_include_directories(Ocean PRIVATE "${CMAKE_SOURCE_DIR}")

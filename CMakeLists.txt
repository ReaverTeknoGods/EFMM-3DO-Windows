# Example CMakeLists.txt for cross-platform build of Escape from Monster Manor
# This demonstrates how to set up the build system for the ported game

cmake_minimum_required(VERSION 3.12)
project(EscapeFromMonsterManor VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build configuration
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_OPENGL "Enable OpenGL rendering" ON)
option(ENABLE_VULKAN "Enable Vulkan rendering" OFF)
option(ENABLE_TESTS "Build unit tests" OFF)

# Add our custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find required packages
find_package(PkgConfig QUIET)

# SDL2 - try to find it, or provide instructions if not found
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found. Creating stub implementation for demonstration.")
    message(STATUS "To build with SDL2 support:")
    message(STATUS "  1. Download SDL2 development libraries from https://www.libsdl.org/")
    message(STATUS "  2. Extract to C:/SDL2/ or set SDL2_DIR environment variable")
    message(STATUS "  3. Re-run cmake")
    set(USE_SDL2_STUBS ON)
else()
    message(STATUS "Found SDL2: ${SDL2_LIBRARIES}")
    set(USE_SDL2_STUBS OFF)
endif()

# SDL2_mixer for audio - only if SDL2 was found
if(NOT USE_SDL2_STUBS)
    find_package(SDL2_mixer QUIET)
    if(NOT SDL2_MIXER_FOUND)
        message(STATUS "SDL2_mixer not found. Audio will use stub implementation.")
        set(USE_AUDIO_STUBS ON)
    else()
        message(STATUS "Found SDL2_mixer: ${SDL2_MIXER_LIBRARIES}")
        set(USE_AUDIO_STUBS OFF)
    endif()
else()
    set(USE_AUDIO_STUBS ON)
endif()

# OpenGL (optional)
if(ENABLE_OPENGL)
    find_package(OpenGL REQUIRED)
    if(OpenGL_FOUND)
        add_definitions(-DENABLE_OPENGL)
    endif()
endif()

# Math library (for trigonometric functions)
find_library(MATH_LIBRARY m)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/platform
    ${CMAKE_SOURCE_DIR}
)

# Add SDL2 include directories if available
if(NOT USE_SDL2_STUBS)
    include_directories(${SDL2_INCLUDE_DIRS})
    if(NOT USE_AUDIO_STUBS)
        include_directories(${SDL2_MIXER_INCLUDE_DIRS})
    endif()
endif()

# Platform-specific source files
if(USE_SDL2_STUBS)
    message(STATUS "Using stub implementations")
    set(PLATFORM_SOURCES
        platform/stub/graphics_stub.c
        platform/stub/audio_stub.c
        platform/stub/input_stub.c
        platform/stub/main_stub.c
    )
    set(PLATFORM_LIBS "")
else()
    message(STATUS "Using SDL2 implementations")
    set(PLATFORM_SOURCES
        platform/sdl/graphics_sdl.c
        platform/sdl/input_sdl.c
        platform/sdl/main_sdl.c
    )
    if(USE_AUDIO_STUBS)
        list(APPEND PLATFORM_SOURCES platform/stub/audio_stub.c)
    else()
        list(APPEND PLATFORM_SOURCES platform/sdl/audio_sdl.c)
    endif()
    
    set(PLATFORM_LIBS ${SDL2_LIBRARIES})
    if(NOT USE_AUDIO_STUBS)
        list(APPEND PLATFORM_LIBS ${SDL2_MIXER_LIBRARIES})
    endif()
    # Add OpenGL for graphics_sdl.c
    if(ENABLE_OPENGL AND OpenGL_FOUND)
        list(APPEND PLATFORM_LIBS ${OPENGL_LIBRARIES})
    endif()
endif()

# Source files - Start with minimal set for initial build
set(CORE_SOURCES
    ctst_ported.c
    game_stubs.c
    cinepak_decode.c
    # Add more files as we port them:
    # imgfile.c (too 3DO-specific, implemented in game_stubs.c)
    # cinepak.c (too 3DO-specific, implemented in game_stubs.c)
    # rend.c
    # shoot.c  
    # objects.c
    # sound.c
    # etc.
)

# All source files
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
)

# SDL2 implementation sources
set(SDL_IMPLEMENTATION_SOURCES
    platform/sdl/graphics_sdl.c
    platform/sdl/audio_sdl.c
    platform/sdl/input_sdl.c
    platform/sdl/main_sdl.c
)

# All sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${SDL_IMPLEMENTATION_SOURCES}
)

# Create the main executable
add_executable(efmm ${ALL_SOURCES})

# Compiler definitions
target_compile_definitions(efmm PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    PLATFORM_SDL2
)

# Compiler flags
if(MSVC)
    target_compile_options(efmm PRIVATE
        /W4 /WX-  # Warning level 4, warnings not as errors
        $<$<CONFIG:Debug>:/Od /Zi>  # No optimization, debug info
        $<$<CONFIG:Release>:/O2>    # Optimize for speed
    )
else()
    target_compile_options(efmm PRIVATE
        -Wall -Wextra -Wno-unused-parameter
        $<$<CONFIG:Debug>:-g -O0>   # Debug info, no optimization
        $<$<CONFIG:Release>:-O3>    # Optimize for speed
    )
endif()

# Link libraries
target_link_libraries(efmm
    ${PLATFORM_LIBS}
    $<$<BOOL:${MATH_LIBRARY}>:${MATH_LIBRARY}>
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(efmm
        winmm      # Windows multimedia
        ws2_32     # Windows sockets
    )
elseif(APPLE)
    target_link_libraries(efmm
        "-framework Cocoa"
        "-framework IOKit" 
        "-framework CoreVideo"
    )
else()
    target_link_libraries(efmm
        pthread    # POSIX threads
        dl         # Dynamic loading
    )
endif()

# Set target properties
set_target_properties(efmm PROPERTIES
    OUTPUT_NAME "EscapeFromMonsterManor"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Video test executable
add_executable(video_test
    video_test.c
    cinepak_decode.c
)

target_include_directories(video_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(video_test
    ${SDL2_LIBRARIES}
)

if(WIN32)
    target_compile_definitions(video_test PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
endif()

set_target_properties(video_test PROPERTIES
    OUTPUT_NAME "VideoTest"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Install rules
install(TARGETS efmm
    RUNTIME DESTINATION bin
)

# Copy game assets to build directory (if they exist)
file(GLOB ASSET_DIRECTORIES 
    "${CMAKE_SOURCE_DIR}/assets/*"
    "${CMAKE_SOURCE_DIR}/levels/*"
    "${CMAKE_SOURCE_DIR}/sounds/*"
    "${CMAKE_SOURCE_DIR}/aiff/*"
)

foreach(ASSET_DIR ${ASSET_DIRECTORIES})
    if(IS_DIRECTORY ${ASSET_DIR})
        get_filename_component(DIR_NAME ${ASSET_DIR} NAME)
        install(DIRECTORY ${ASSET_DIR}
            DESTINATION share/efmm/${DIR_NAME}
        )
    endif()
endforeach()

# Development tools
if(ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "Escape from Monster Manor")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Classic 3DO horror game ported to modern systems")
set(CPACK_PACKAGE_VENDOR "Community Port")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "Escape from Monster Manor - Build Configuration Summary")
message(STATUS "========================================================")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "C Compiler:      ${CMAKE_C_COMPILER}")
message(STATUS "Install prefix:  ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  OpenGL:        ${ENABLE_OPENGL}")
message(STATUS "  Vulkan:        ${ENABLE_VULKAN}")
message(STATUS "  Tests:         ${ENABLE_TESTS}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  SDL2:          ${SDL2_FOUND}")
message(STATUS "  SDL2_mixer:    ${SDL2_MIXER_FOUND}")
message(STATUS "  OpenGL:        ${OpenGL_FOUND}")
message(STATUS "")

# FindSDL2_mixer.cmake
# Locate SDL2_mixer library
# This module defines
#  SDL2_MIXER_FOUND - System has SDL2_mixer
#  SDL2_MIXER_INCLUDE_DIRS - The SDL2_mixer include directories
#  SDL2_MIXER_LIBRARIES - The libraries needed to use SDL2_mixer

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SDL2_MIXER QUIET SDL2_mixer)
endif()

find_path(SDL2_MIXER_INCLUDE_DIR
    NAMES SDL_mixer.h
    HINTS
        ${PC_SDL2_MIXER_INCLUDEDIR}
        ${PC_SDL2_MIXER_INCLUDE_DIRS}
        $ENV{SDL2MIXERDIR}
        $ENV{SDL2_MIXER_DIR}
    PATH_SUFFIXES
        include
        include/SDL2
        SDL2
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include
        /opt/local/include
        /opt/csw/include
        /opt/include
        "C:/SDL2_mixer/include"
        "C:/SDL2_mixer-2.8.1/include"
        "C:/SDL2_mixer-2.*/include"
        "C:/Program Files/SDL2_mixer/include"
        "C:/Program Files (x86)/SDL2_mixer/include"
        "${CMAKE_SOURCE_DIR}/third_party/SDL2_mixer/include"
        "${CMAKE_SOURCE_DIR}/SDL2_mixer-2.8.1/include"
        "C:/work/EFMM-3DO-main/SDL2_mixer-2.8.1/include"
)

find_library(SDL2_MIXER_LIBRARY
    NAMES SDL2_mixer
    HINTS
        ${PC_SDL2_MIXER_LIBDIR}
        ${PC_SDL2_MIXER_LIBRARY_DIRS}
        $ENV{SDL2MIXERDIR}
        $ENV{SDL2_MIXER_DIR}
    PATH_SUFFIXES
        lib
        lib64
        lib/x64
        lib/x86
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/SDL2_mixer/lib"
        "C:/SDL2_mixer-2.8.1/lib"
        "C:/SDL2_mixer-2.*/lib"
        "C:/Program Files/SDL2_mixer/lib"
        "C:/Program Files (x86)/SDL2_mixer/lib"
        "${CMAKE_SOURCE_DIR}/third_party/SDL2_mixer/lib"
        "${CMAKE_SOURCE_DIR}/SDL2_mixer-2.8.1/lib"
        "C:/work/EFMM-3DO-main/SDL2_mixer-2.8.1/lib"
        "C:/work/EFMM-3DO-main/SDL2_mixer-2.8.1/lib/x64"
)

if(SDL2_MIXER_INCLUDE_DIR AND SDL2_MIXER_LIBRARY)
    set(SDL2_MIXER_FOUND TRUE)
    set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
    set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
    
    # Create imported target
    if(NOT TARGET SDL2::mixer)
        add_library(SDL2::mixer UNKNOWN IMPORTED)
        set_target_properties(SDL2::mixer PROPERTIES
            IMPORTED_LOCATION "${SDL2_MIXER_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_MIXER_INCLUDE_DIRS}"
        )
    endif()
else()
    set(SDL2_MIXER_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer
    FOUND_VAR SDL2_MIXER_FOUND
    REQUIRED_VARS SDL2_MIXER_LIBRARY SDL2_MIXER_INCLUDE_DIR
)

mark_as_advanced(SDL2_MIXER_INCLUDE_DIR SDL2_MIXER_LIBRARY)

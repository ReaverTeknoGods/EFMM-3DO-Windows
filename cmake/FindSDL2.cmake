# FindSDL2.cmake
# Locate SDL2 library
# This module defines
#  SDL2_FOUND - System has SDL2
#  SDL2_INCLUDE_DIRS - The SDL2 include directories
#  SDL2_LIBRARIES - The libraries needed to use SDL2
#  SDL2_DEFINITIONS - Compiler switches required for using SDL2

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SDL2 QUIET sdl2)
endif()

# Look for SDL2 in common locations
find_path(SDL2_INCLUDE_DIR
    NAMES SDL.h
    HINTS
        ${PC_SDL2_INCLUDEDIR}
        ${PC_SDL2_INCLUDE_DIRS}
        $ENV{SDL2DIR}
        $ENV{SDL2_DIR}
    PATH_SUFFIXES
        include
        include/SDL2
        SDL2
        ""
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/include
        /usr/include
        /sw/include
        /opt/local/include
        /opt/csw/include
        /opt/include
        "C:/SDL2/include"
        "C:/SDL2-2.32.10/include"
        "C:/SDL2-2.0.*/include"
        "C:/Program Files/SDL2/include"
        "C:/Program Files (x86)/SDL2/include"
        "${CMAKE_SOURCE_DIR}/third_party/SDL2/include"
        "${CMAKE_SOURCE_DIR}/SDL2-2.32.10/include"
        "C:/work/EFMM-3DO-main/SDL2-2.32.10/include"
)

find_library(SDL2_LIBRARY
    NAMES SDL2 SDL2main
    HINTS
        ${PC_SDL2_LIBDIR}
        ${PC_SDL2_LIBRARY_DIRS}
        $ENV{SDL2DIR}
        $ENV{SDL2_DIR}
    PATH_SUFFIXES
        lib
        lib64
        lib/x64
        lib/x86
        x64
        x86
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/SDL2/lib"
        "C:/SDL2-2.32.10/lib"
        "C:/SDL2-2.0.*/lib"
        "C:/Program Files/SDL2/lib"
        "C:/Program Files (x86)/SDL2/lib"
        "${CMAKE_SOURCE_DIR}/third_party/SDL2/lib"
        "${CMAKE_SOURCE_DIR}/SDL2-2.32.10/lib"
        "C:/work/EFMM-3DO-main/SDL2-2.32.10/lib"
        "C:/work/EFMM-3DO-main/SDL2-2.32.10/lib/x64"
)

find_library(SDL2_MAIN_LIBRARY
    NAMES SDL2main
    HINTS
        ${PC_SDL2_LIBDIR}
        ${PC_SDL2_LIBRARY_DIRS}
        $ENV{SDL2DIR}
        $ENV{SDL2_DIR}
    PATH_SUFFIXES
        lib
        lib64
        lib/x64
        lib/x86
        x64
        x86
    PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local/lib
        /usr/lib
        /sw/lib
        /opt/local/lib
        /opt/csw/lib
        /opt/lib
        "C:/SDL2/lib"
        "C:/SDL2-2.32.10/lib"
        "C:/SDL2-2.0.*/lib"
        "C:/Program Files/SDL2/lib"
        "C:/Program Files (x86)/SDL2/lib"
        "${CMAKE_SOURCE_DIR}/third_party/SDL2/lib"
        "${CMAKE_SOURCE_DIR}/SDL2-2.32.10/lib"
        "C:/work/EFMM-3DO-main/SDL2-2.32.10/lib"
        "C:/work/EFMM-3DO-main/SDL2-2.32.10/lib/x64"
)

if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
    set(SDL2_FOUND TRUE)
    
    set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    
    if(SDL2_MAIN_LIBRARY)
        set(SDL2_LIBRARIES ${SDL2_MAIN_LIBRARY} ${SDL2_LIBRARY})
    else()
        set(SDL2_LIBRARIES ${SDL2_LIBRARY})
    endif()
    
    # Add system libraries on Windows
    if(WIN32)
        list(APPEND SDL2_LIBRARIES winmm imm32 ole32 oleaut32 version uuid advapi32 setupapi shell32)
    endif()
    
    if(PC_SDL2_CFLAGS_OTHER)
        set(SDL2_DEFINITIONS ${PC_SDL2_CFLAGS_OTHER})
    endif()
    
    # Create imported target
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            IMPORTED_LOCATION "${SDL2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
        )
        
        if(WIN32)
            set_target_properties(SDL2::SDL2 PROPERTIES
                INTERFACE_LINK_LIBRARIES "winmm;imm32;ole32;oleaut32;version;uuid;advapi32;setupapi;shell32"
            )
        endif()
    endif()
    
    if(SDL2_MAIN_LIBRARY AND NOT TARGET SDL2::SDL2main)
        add_library(SDL2::SDL2main UNKNOWN IMPORTED)
        set_target_properties(SDL2::SDL2main PROPERTIES
            IMPORTED_LOCATION "${SDL2_MAIN_LIBRARY}"
        )
    endif()
else()
    set(SDL2_FOUND FALSE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2
    FOUND_VAR SDL2_FOUND
    REQUIRED_VARS SDL2_LIBRARY SDL2_INCLUDE_DIR
    VERSION_VAR SDL2_VERSION_STRING
)

mark_as_advanced(SDL2_INCLUDE_DIR SDL2_LIBRARY SDL2_MAIN_LIBRARY)

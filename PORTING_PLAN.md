# Escape from Monster Manor - Cross-Platform Porting Plan

## Overview
This document outlines the strategy for porting the 3DO game "Escape from Monster Manor" to modern cross-platform systems (Windows, macOS, Linux).

## Phase 1: Platform Abstraction Layer (PAL)

### 1.1 Create Abstract Interface Headers
```c
// platform/platform_types.h - Replace 3DO types
typedef float frac16_float;  // Replace frac16 with float
typedef uint32_t Item;       // Replace 3DO Item type
typedef struct Vertex3D {
    float X, Y, Z;
} Vertex3D;

// platform/platform_graphics.h - Graphics abstraction
typedef struct PlatformTexture {
    void* data;
    int width, height;
    int format;
} PlatformTexture;

typedef struct PlatformRenderer {
    void* renderer_data;
    // Function pointers for different backends
    int (*draw_texture)(PlatformTexture* tex, float x, float y);
    int (*clear_screen)(uint32_t color);
} PlatformRenderer;

// platform/platform_audio.h - Audio abstraction  
typedef struct PlatformSound {
    void* audio_data;
    int length;
    int sample_rate;
} PlatformSound;

// platform/platform_input.h - Input abstraction
typedef struct PlatformInput {
    int keys[256];
    int mouse_x, mouse_y;
    int mouse_buttons;
} PlatformInput;
```

### 1.2 Replace 3DO-Specific Types
- Convert `frac16` fixed-point to `float` or keep fixed-point with portable implementation
- Replace `CCB` (Cel Control Block) with generic sprite/texture structure
- Replace `RastPort` with generic render target
- Replace 3DO memory types with standard allocation

## Phase 2: Core System Replacements

### 2.1 Graphics System (SDL2 + OpenGL recommended)
```c
// platform/sdl/graphics_sdl.c
int platform_init_graphics(int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
    // Create window and OpenGL context
    return 0;
}

int platform_draw_cel(PlatformTexture* tex, float x, float y, float scale) {
    // Convert 3DO cel drawing to OpenGL texture rendering
    return 0;
}

int platform_clear_screen(uint32_t floor_color, uint32_t ceiling_color) {
    // Replace SetVRAMPages with OpenGL clear
    return 0;
}
```

### 2.2 Audio System (SDL_mixer recommended)
```c
// platform/sdl/audio_sdl.c  
int platform_init_audio() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) return -1;
    return 0;
}

int platform_play_sound(int sound_id) {
    // Replace 3DO sound calls with SDL_mixer
    return 0;
}

int platform_load_sound(const char* filename) {
    // Load AIFC/AIFF files and convert if needed
    return 0;
}
```

### 2.3 Input System
```c
// platform/sdl/input_sdl.c
int platform_update_input(PlatformInput* input) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Convert SDL events to platform input
    }
    return 0;
}
```

### 2.4 File System
```c
// platform/filesystem.c
int platform_load_file(const char* filename, void** data, size_t* size) {
    // Replace 3DO file loading with standard C file I/O
    return 0;
}
```

## Phase 3: Mathematics Conversion

### 3.1 Fixed-Point to Floating-Point
Option A: Convert all math to floating-point
```c
// Replace throughout codebase:
// frac16 -> float
// ONE_F16 -> 1.0f  
// HALF_F16 -> 0.5f
// MulSF16(a,b) -> (a * b)
// DivSF16(a,b) -> (a / b)
```

Option B: Keep fixed-point with portable implementation
```c
// math/fixed_point.h
typedef int32_t frac16;
#define ONE_F16 65536
#define HALF_F16 32768

static inline frac16 MulSF16(frac16 a, frac16 b) {
    return (frac16)(((int64_t)a * b) >> 16);
}

static inline frac16 DivSF16(frac16 a, frac16 b) {
    return (frac16)(((int64_t)a << 16) / b);
}
```

## Phase 4: Asset Conversion

### 4.1 Image Assets
- Convert 3DO .cel files to PNG/TGA
- Extract color palettes and convert to RGB
- Rebuild sprite/texture loading system

### 4.2 Audio Assets  
- Convert AIFC files to WAV/OGG
- Maintain music streaming for large files
- Convert any 3DO-specific audio effects

### 4.3 Level Data
- Convert level map files to portable format
- Ensure object definitions work with new system

## Phase 5: Build System

### 5.1 CMake Build System
```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.12)
project(EscapeFromMonsterManor)

# Find packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2_mixer REQUIRED)

# Source files
set(SOURCES
    src/ctst.c
    src/rend.c
    src/objects.c
    # ... all other source files
    
    # Platform abstraction
    platform/graphics.c
    platform/audio.c
    platform/input.c
    platform/filesystem.c
)

# Platform-specific sources
if(WIN32)
    list(APPEND SOURCES platform/windows/platform_windows.c)
elseif(APPLE)
    list(APPEND SOURCES platform/macos/platform_macos.c)
else()
    list(APPEND SOURCES platform/linux/platform_linux.c)
endif()

add_executable(efmm ${SOURCES})
target_link_libraries(efmm SDL2::SDL2 SDL2_mixer::SDL2_mixer OpenGL::GL)
```

## Phase 6: Testing and Debugging

### 6.1 Incremental Testing
1. Start with basic initialization and graphics
2. Add input handling
3. Add basic audio playback
4. Integrate game logic step by step
5. Add asset loading
6. Full game testing

### 6.2 Debug Tools
- Add modern debugging support
- Frame rate monitoring
- Memory debugging with standard tools
- Cross-platform testing

## Estimated Effort
- **Phase 1-2**: 2-3 weeks (Platform abstraction)
- **Phase 3**: 1-2 weeks (Math conversion) 
- **Phase 4**: 2-3 weeks (Asset conversion)
- **Phase 5**: 1 week (Build system)
- **Phase 6**: 2-4 weeks (Testing and polish)

**Total: 8-13 weeks** for a complete port

## Key Challenges
1. **3D Graphics**: The game uses custom 3D rendering that may need significant rework
2. **Performance**: Original game was highly optimized for 3DO hardware
3. **Assets**: Missing level data and art assets mentioned in README
4. **Fixed-Point Math**: Extensive use throughout the codebase
5. **Custom File Formats**: LOAF format and other proprietary formats

## Recommended Libraries
- **Graphics**: SDL2 + OpenGL 3.3+ or SDL2 + Vulkan
- **Audio**: SDL2_mixer or OpenAL
- **Input**: SDL2 (built-in)
- **Math**: GLM (if converting to floating-point)
- **Build**: CMake for cross-platform builds
- **Testing**: Unity test framework or similar

## Success Criteria
- Game runs at 60fps on modern hardware
- All original gameplay mechanics preserved
- Cross-platform compatibility (Windows, macOS, Linux)
- Modern input support (keyboard, mouse, gamepad)
- Asset loading from common formats
- Maintainable, well-documented codebase

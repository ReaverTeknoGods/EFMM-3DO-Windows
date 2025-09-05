#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/*
 * Cross-platform type definitions to replace 3DO-specific types
 * This file replaces the 3DO types.h and provides portable equivalents
 */

// Basic types (replacing 3DO types.h)
typedef uint8_t   ubyte;
typedef int8_t    int8;
typedef uint16_t  uint16;
typedef int16_t   int16;
typedef uint32_t  uint32;
typedef int32_t   int32;
typedef uint64_t  uint64;
typedef int64_t   int64;

// Fixed-point arithmetic (replacing operamath.h)
typedef int32_t   frac16;
#define ONE_F16   (1 << 16)
#define HALF_F16  (1 << 15)

// Fixed-point math functions
static inline frac16 MulSF16(frac16 a, frac16 b) {
    return (frac16)(((int64_t)a * b) >> 16);
}

static inline frac16 DivSF16(frac16 a, frac16 b) {
    return (frac16)(((int64_t)a << 16) / b);
}

static inline frac16 SinF16(frac16 angle) {
    // Simple sin approximation - replace with proper implementation
    float rad = (float)angle / ONE_F16;
    return (frac16)(sin(rad) * ONE_F16);
}

static inline frac16 CosF16(frac16 angle) {
    // Simple cos approximation - replace with proper implementation  
    float rad = (float)angle / ONE_F16;
    return (frac16)(cos(rad) * ONE_F16);
}

static inline frac16 SquareSF16(frac16 a) {
    return MulSF16(a, a);
}

static inline frac16 Convert32_F16(int32 val) {
    return val << 16;
}

static inline int32 ConvertF16_32(frac16 val) {
    return val >> 16;
}

// Utility macros
#define F_INT(x)    ((x) & ~0xFFFF)
#define F_FRAC(x)   ((x) & 0xFFFF)

// 3D math structures (replacing 3DO vector types)
typedef struct Vertex {
    frac16 X, Y, Z;
} Vertex;

typedef struct Vector {
    frac16 X, Y, Z;
} Vector;

typedef struct Matrix {
    frac16 X0, Y0, Z0,
           X1, Y1, Z1,
           X2, Y2, Z2;
} Matrix;

// Bounding box for collision detection
typedef struct BBox {
    frac16 MinX, MinZ, MaxX, MaxZ;
} BBox;

typedef struct PathBox {
    BBox Start, End, Path;
    frac16 DX, DZ;
} PathBox;

// 2D point structure
typedef struct Point {
    int32 pt_X, pt_Y;
} Point;

// Color definitions (replacing 3DO color types)
typedef uint32 Color;

#define MakeRGB15(r,g,b) (((r) << 10) | ((g) << 5) | (b))
#define MakeRGB15Pair(r,g,b) (MakeRGB15(r,g,b) | (MakeRGB15(r,g,b) << 16))
#define MakeCLUTColorEntry(idx,r,g,b) (((idx) << 24) | ((r) << 16) | ((g) << 8) | (b))

// Memory allocation types (replacing 3DO MEMTYPE flags)
#define MEMTYPE_NORMAL    0x00000001
#define MEMTYPE_VRAM      0x00000002  
#define MEMTYPE_CEL       0x00000004
#define MEMTYPE_FILL      0x00000008
#define MEMTYPE_DMA       0x00000010

// Item type (3DO uses this extensively)
typedef int32 Item;

// Forward declarations for platform-specific structures
struct PlatformTexture;
struct PlatformSound;
struct PlatformRenderer;

#endif // PLATFORM_TYPES_H

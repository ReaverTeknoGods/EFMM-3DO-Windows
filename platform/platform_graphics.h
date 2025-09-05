#ifndef PLATFORM_GRAPHICS_H
#define PLATFORM_GRAPHICS_H

#include "platform_types.h"

/*
 * Cross-platform graphics abstraction
 * Replaces 3DO graphics.h functionality
 */

// Forward declarations
typedef struct CCB CCB;
typedef struct CelArray CelArray;
typedef struct RastPort RastPort;
typedef struct ScreenItem ScreenItem;

// Platform-specific texture structure (shared between modules)
typedef struct PlatformTexture {
    void* gl_id;      // OpenGL texture ID (or other platform-specific ID)
    int width;        // Texture width
    int height;       // Texture height  
    int format;       // Pixel format (4 = RGBA32)
    void* data;       // Pixel data buffer
} PlatformTexture;

// Graphics context structure (replaces 3DO GrafCon)
typedef struct GrafCon {
    Color fg_color;
    Color bg_color;
    // Additional state as needed
} GrafCon;

// Bitmap structure (replaces 3DO Bitmap)
typedef struct Bitmap {
    void* bm_Buffer;
    int32 bm_Width;
    int32 bm_Height;
    int32 bm_BytesPerRow;
    uint32 bm_Format;
} Bitmap;

// Screen structure (replaces 3DO Screen)
typedef struct Screen {
    Item sc_ScreenItem;
    Bitmap* sc_Bitmap;
    int32 sc_Width;
    int32 sc_Height;
} Screen;

// RastPort structure (replaces 3DO RastPort)
typedef struct RastPort {
    Item rp_ScreenItem;
    Item rp_BitmapItem;
    Bitmap* rp_Bitmap;
    void* rp_ScreenPtr;  // Platform-specific screen pointer
} RastPort;

// Cel Control Block structure (replaces 3DO CCB)
typedef struct CCB {
    struct CCB* ccb_NextPtr;
    void* ccb_SourcePtr;
    void* ccb_PLUTPtr;
    
    // Positioning and scaling
    frac16 ccb_XPos;
    frac16 ccb_YPos;
    frac16 ccb_HDX;   // Horizontal delta X
    frac16 ccb_HDY;   // Horizontal delta Y  
    frac16 ccb_VDX;   // Vertical delta X
    frac16 ccb_VDY;   // Vertical delta Y
    
    // Cel dimensions
    int32 ccb_Width;
    int32 ccb_Height;
    
    // Control flags
    uint32 ccb_Flags;
    uint32 ccb_PIXC;  // Pixel processor control
    
    // Platform-specific texture data
    struct PlatformTexture* platform_texture;
} CCB;

// CCB Flags (from 3DO)
#define CCB_SKIP        0x80000000
#define CCB_LAST        0x40000000  
#define CCB_NPABS       0x20000000
#define CCB_SPABS       0x10000000
#define CCB_PPABS       0x08000000
#define CCB_LDSIZE      0x04000000
#define CCB_LDPRS       0x02000000
#define CCB_LDPPMP      0x01000000
#define CCB_LDPLUT      0x00800000
#define CCB_CCBPRE      0x00400000
#define CCB_YOXY        0x00200000
#define CCB_ACSC        0x00100000
#define CCB_ALSC        0x00080000
#define CCB_ACW         0x00040000
#define CCB_ACCW        0x00020000
#define CCB_TWD         0x00010000

// Cel Array structure (replaces 3DO CelArray)
typedef struct CelArray {
    int32 ca_nCCBs;
    CCB** celptrs;
} CelArray;

// Graphics initialization and cleanup
int platform_init_graphics(int width, int height, bool fullscreen);
void platform_shutdown_graphics(void);

// Screen management (replaces 3DO screen functions)
int CreateScreenGroup(Item* screen_items, void* tags);
int DisplayScreen(Item screen_item, uint32 value);
int DeleteScreenGroup(Item* screen_items);

// Drawing functions (replaces 3DO drawing functions)
int DrawCels(Item bitmap_item, CCB* ccb);
int SetRast(RastPort* rp, uint32 color);
int CopyVRAMPages(Item vram_io, void* dest, void* src, int32 pages, uint32 mask);
int SetVRAMPages(Item vram_io, void* dest, uint32 value, int32 pages, uint32 mask);

// Color and palette functions
int SetScreenColor(Item screen_item, uint32 color_entry);

// VBL and timing (replaces 3DO timing functions)
int GetVBLIOReq(void);
int WaitVBL(Item vbl_io, int32 frames);

// Memory management for graphics
void* AllocMem(int32 size, uint32 mem_flags);
void FreeMem(void* ptr, int32 size);

// Utility functions
void FasterMapCel(CCB* ccb, Point* corners);
void SetFGPen(GrafCon* gc, Color color);
void WritePixel(Item bitmap_item, GrafCon* gc, int32 x, int32 y);

// Platform-specific graphics base structure
typedef struct GraphicsBase {
    int32 gf_VRAMPageSize;
    // Other platform-specific fields
} GraphicsBase;

extern GraphicsBase* GrafBase;

// Function to open graphics subsystem
int OpenGraphicsFolio(void);
void CloseGraphicsFolio(void);

// Bitmap access function (for video frame rendering)
Bitmap* get_bitmap(Item bitmap_item);

#endif // PLATFORM_GRAPHICS_H

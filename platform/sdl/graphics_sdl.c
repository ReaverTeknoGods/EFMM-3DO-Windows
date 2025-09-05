#include "platform/platform_graphics.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * SDL2 + OpenGL graphics implementation
 * Replaces 3DO graphics system with modern cross-platform equivalent
 */

// Global state
static SDL_Window* g_window = NULL;
static SDL_GLContext g_gl_context = NULL;
static SDL_Renderer* g_renderer = NULL;
static bool g_use_opengl = false; // Temporarily disable OpenGL to test SDL2 renderer
static int g_screen_width = 320;
static int g_screen_height = 240;
static int g_window_width = 640;
static int g_window_height = 480;

// Simulated 3DO structures
static GraphicsBase g_graphics_base = {0};
GraphicsBase* GrafBase = &g_graphics_base;

// Screen items simulation
static Screen g_screens[4];
static RastPort g_rastports[4];
static Bitmap g_bitmaps[4];
static int g_num_screens = 0;

// VBL timing
static Uint32 g_last_vbl_time = 0;
static const int VBL_RATE = 60; // 60 Hz

// Graphics initialization
int platform_init_graphics(int width, int height, bool fullscreen)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    g_screen_width = width;
    g_screen_height = height;
    g_window_width = width * 2;  // 2x scaling by default
    g_window_height = height * 2;

    // Create window
    Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (fullscreen) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if (g_use_opengl) {
        // Set OpenGL attributes
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        
        window_flags |= SDL_WINDOW_OPENGL;
    }

    g_window = SDL_CreateWindow(
        "Escape from Monster Manor",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        g_window_width,
        g_window_height,
        window_flags
    );

    if (!g_window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    if (g_use_opengl) {
        // Create OpenGL context
        g_gl_context = SDL_GL_CreateContext(g_window);
        if (!g_gl_context) {
            printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(g_window);
            SDL_Quit();
            return -1;
        }

        // Enable VSync
        SDL_GL_SetSwapInterval(1);

        // Initialize OpenGL
        glViewport(0, 0, g_window_width, g_window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    } else {
        // Create renderer for 2D fallback
        g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!g_renderer) {
            printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(g_window);
            SDL_Quit();
            return -1;
        }
    }

    // Initialize graphics base
    g_graphics_base.gf_VRAMPageSize = 2048; // Simulate 3DO VRAM page size

    printf("Graphics initialized: %dx%d (window: %dx%d)\n", 
           g_screen_width, g_screen_height, g_window_width, g_window_height);

    return 0;
}

void platform_shutdown_graphics(void)
{
    if (g_gl_context) {
        SDL_GL_DeleteContext(g_gl_context);
        g_gl_context = NULL;
    }

    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
        g_renderer = NULL;
    }

    if (g_window) {
        SDL_DestroyWindow(g_window);
        g_window = NULL;
    }

    SDL_Quit();
}

// 3DO compatibility functions
int OpenGraphicsFolio(void)
{
    // Already initialized in platform_init_graphics
    return 0;
}

void CloseGraphicsFolio(void)
{
    // Will be handled in platform_shutdown_graphics
}

int CreateScreenGroup(Item* screen_items, void* tags)
{
    // Create simulated screens
    for (int i = 0; i < 2; i++) {
        // Initialize bitmap
        g_bitmaps[i].bm_Buffer = malloc(g_screen_width * g_screen_height * 4); // RGBA
        g_bitmaps[i].bm_Width = g_screen_width;
        g_bitmaps[i].bm_Height = g_screen_height;
        g_bitmaps[i].bm_BytesPerRow = g_screen_width * 4;
        g_bitmaps[i].bm_Format = 0; // RGBA

        // Initialize screen
        g_screens[i].sc_ScreenItem = i + 1;
        g_screens[i].sc_Bitmap = &g_bitmaps[i];
        g_screens[i].sc_Width = g_screen_width;
        g_screens[i].sc_Height = g_screen_height;

        // Initialize rastport
        g_rastports[i].rp_ScreenItem = i + 1;
        g_rastports[i].rp_BitmapItem = i + 1;
        g_rastports[i].rp_Bitmap = &g_bitmaps[i];
        g_rastports[i].rp_ScreenPtr = &g_screens[i];

        screen_items[i] = i + 1;
    }

    g_num_screens = 2;
    return 0;
}

// Get platform rastport by index
RastPort* platform_get_rastport(int index) {
    if (index >= 0 && index < g_num_screens) {
        return &g_rastports[index];
    }
    return NULL;
}

int DisplayScreen(Item screen_item, uint32 value)
{
    printf("=== DisplayScreen() called: item=%d, value=%u ===\n", screen_item, value);
    
    // Find the screen
    if (screen_item < 1 || screen_item > g_num_screens) {
        printf("ERROR: Invalid screen_item %d (valid range: 1-%d)\n", screen_item, g_num_screens);
        return -1;
    }

    Screen* screen = &g_screens[screen_item - 1];
    printf("Screen: %p, bitmap: %p, dimensions: %dx%d\n", 
           screen, screen->sc_Bitmap, screen->sc_Width, screen->sc_Height);
    
    if (!screen->sc_Bitmap || !screen->sc_Bitmap->bm_Buffer) {
        printf("ERROR: Screen bitmap or buffer is NULL\n");
        return -1;
    }
    
    printf("Buffer: %p, BytesPerRow: %d\n", 
           screen->sc_Bitmap->bm_Buffer, screen->sc_Bitmap->bm_BytesPerRow);
    
    if (g_use_opengl) {
        printf("Using OpenGL rendering...\n");
        // OpenGL rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Upload screen buffer as texture and render
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen->sc_Width, screen->sc_Height, 
                     0, GL_RGBA, GL_UNSIGNED_BYTE, screen->sc_Bitmap->bm_Buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // Render fullscreen quad (simplified)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(-1, -1);
        glTexCoord2f(1, 1); glVertex2f(1, -1);
        glTexCoord2f(1, 0); glVertex2f(1, 1);
        glTexCoord2f(0, 0); glVertex2f(-1, 1);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glDeleteTextures(1, &texture);
        SDL_GL_SwapWindow(g_window);
        printf("OpenGL frame rendered and swapped\n");

    } else {
        printf("Using SDL2 renderer fallback...\n");
        // SDL2 renderer fallback
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
        SDL_RenderClear(g_renderer);
        
        // Create texture from screen buffer
        SDL_Texture* texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGBA32,
                                                SDL_TEXTUREACCESS_STREAMING,
                                                screen->sc_Width, screen->sc_Height);
        if (texture) {
            printf("Created SDL texture, updating with buffer data...\n");
            
            // Debug: Check first few pixels to see if we have actual data
            uint32_t* pixels = (uint32_t*)screen->sc_Bitmap->bm_Buffer;
            printf("First 8 pixels: 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X\n",
                   pixels[0], pixels[1], pixels[2], pixels[3], pixels[4], pixels[5], pixels[6], pixels[7]);
            
            SDL_UpdateTexture(texture, NULL, screen->sc_Bitmap->bm_Buffer, 
                            screen->sc_Bitmap->bm_BytesPerRow);
            SDL_RenderCopy(g_renderer, texture, NULL, NULL);
            SDL_DestroyTexture(texture);
            printf("Texture rendered to screen\n");
        } else {
            printf("ERROR: Failed to create SDL texture\n");
        }
        
        SDL_RenderPresent(g_renderer);
        printf("SDL frame presented\n");
    }

    printf("=== DisplayScreen() completed ===\n");
    return 0;
}

int DeleteScreenGroup(Item* screen_items)
{
    for (int i = 0; i < g_num_screens; i++) {
        if (g_bitmaps[i].bm_Buffer) {
            free(g_bitmaps[i].bm_Buffer);
            g_bitmaps[i].bm_Buffer = NULL;
        }
    }
    g_num_screens = 0;
    return 0;
}

// Drawing functions
int DrawCels(Item bitmap_item, CCB* ccb)
{
    printf("=== DrawCels() Entry ===\n");
    printf("bitmap_item=%d, ccb=%p\n", bitmap_item, ccb);
    
    if (!ccb) {
        printf("ERROR: ccb is NULL\n");
        return -1;
    }
    
    if (bitmap_item < 1 || bitmap_item > g_num_screens) {
        printf("ERROR: Invalid bitmap_item %d (valid range: 1-%d)\n", bitmap_item, g_num_screens);
        return -1;
    }

    printf("Getting bitmap for item %d\n", bitmap_item);
    Bitmap* bitmap = &g_bitmaps[bitmap_item - 1];
    printf("bitmap=%p, bm_Buffer=%p, dimensions=%dx%d\n", 
           bitmap, bitmap->bm_Buffer, bitmap->bm_Width, bitmap->bm_Height);
    
    if (!bitmap->bm_Buffer) {
        printf("ERROR: bitmap buffer is NULL\n");
        return -1;
    }
    
    uint32_t* pixels = (uint32_t*)bitmap->bm_Buffer;
    printf("pixels=%p\n", pixels);

    // Iterate through cel chain
    CCB* current = ccb;
    int cel_count = 0;
    
    while (current) {
        cel_count++;
        printf("\n--- Processing cel #%d ---\n", cel_count);
        printf("current=%p, Flags=0x%08X\n", current, current->ccb_Flags);
        
        if (!(current->ccb_Flags & CCB_SKIP)) {
            printf("Cel not skipped, checking texture data...\n");
            
            // Simple cel rendering - convert cel to bitmap pixels
            if (!current->platform_texture) {
                printf("WARNING: platform_texture is NULL\n");
                goto next_cel;
            }
            
            printf("platform_texture=%p\n", current->platform_texture);
            
            if (!current->platform_texture->data) {
                printf("WARNING: platform_texture->data is NULL\n");
                goto next_cel;
            }
            
            printf("texture data=%p\n", current->platform_texture->data);
            
            // Render cel at specified position
            int cel_x = ConvertF16_32(current->ccb_XPos);
            int cel_y = ConvertF16_32(current->ccb_YPos);
            
            printf("Cel position: (%d,%d)\n", cel_x, cel_y);
            
            // Simple blit operation (needs proper transformation)
            uint32_t* cel_data = (uint32_t*)current->platform_texture->data;
            int cel_width = current->ccb_Width;
            int cel_height = current->ccb_Height;
            
            printf("Cel dimensions: %dx%d\n", cel_width, cel_height);
            printf("cel_data=%p\n", cel_data);
            
            // Safety checks
            if (cel_width <= 0 || cel_height <= 0) {
                printf("ERROR: Invalid cel dimensions: %dx%d\n", cel_width, cel_height);
                goto next_cel;
            }
            
            if (cel_width > 1000 || cel_height > 1000) {
                printf("ERROR: Suspicious cel dimensions: %dx%d (too large)\n", cel_width, cel_height);
                goto next_cel;
            }
            
            printf("Starting pixel copy loop...\n");
            printf("Target bitmap: %dx%d\n", bitmap->bm_Width, bitmap->bm_Height);
            
            int pixels_copied = 0;
            
            for (int y = 0; y < cel_height && (cel_y + y) < bitmap->bm_Height; y++) {
                if (y % 20 == 0) { // Log every 20th row
                    printf("Processing row %d/%d\n", y, cel_height);
                }
                
                for (int x = 0; x < cel_width && (cel_x + x) < bitmap->bm_Width; x++) {
                    if (cel_x + x >= 0 && cel_y + y >= 0) {
                        int dst_idx = (cel_y + y) * bitmap->bm_Width + (cel_x + x);
                        int src_idx = y * cel_width + x;
                        
                        // Enhanced safety checks
                        int max_dst = bitmap->bm_Width * bitmap->bm_Height;
                        int max_src = cel_width * cel_height;
                        
                        if (dst_idx >= max_dst) {
                            printf("ERROR: dst_idx %d >= max_dst %d (x=%d,y=%d)\n", 
                                   dst_idx, max_dst, x, y);
                            goto next_cel;
                        }
                        
                        if (src_idx >= max_src) {
                            printf("ERROR: src_idx %d >= max_src %d (x=%d,y=%d)\n", 
                                   src_idx, max_src, x, y);
                            goto next_cel;
                        }
                        
                        // Test memory access before actual copy
                        volatile uint32_t test_read;
                        volatile uint32_t* test_src = &cel_data[src_idx];
                        volatile uint32_t* test_dst = &pixels[dst_idx];
                        
                        // Try to read source
                        test_read = *test_src;
                        
                        // Try to write destination
                        *test_dst = test_read;
                        
                        pixels_copied++;
                        
                        if (pixels_copied % 1000 == 0) { // Log every 1000 pixels
                            printf("Copied %d pixels so far...\n", pixels_copied);
                        }
                    }
                }
            }
            printf("Pixel copy completed! Copied %d pixels total\n", pixels_copied);
            
        } else {
            printf("Cel skipped due to CCB_SKIP flag\n");
        }

        next_cel:
        printf("Moving to next cel...\n");
        
        if (current->ccb_Flags & CCB_LAST) {
            printf("Found CCB_LAST flag, ending cel chain\n");
            break;
        }
        
        printf("ccb_NextPtr=%p\n", current->ccb_NextPtr);
        current = current->ccb_NextPtr;
        
        if (cel_count > 10) {
            printf("WARNING: Processed more than 10 cels, breaking to prevent infinite loop\n");
            break;
        }
    }

    printf("=== DrawCels() Exit (success) ===\n");
    return 0;
}

int SetRast(RastPort* rp, uint32 color)
{
    if (!rp || !rp->rp_Bitmap || !rp->rp_Bitmap->bm_Buffer) {
        return -1;
    }

    uint32_t* pixels = (uint32_t*)rp->rp_Bitmap->bm_Buffer;
    int total_pixels = rp->rp_Bitmap->bm_Width * rp->rp_Bitmap->bm_Height;

    // Convert color to RGBA
    uint32_t rgba_color = 0xFF000000 | color; // Add alpha

    for (int i = 0; i < total_pixels; i++) {
        pixels[i] = rgba_color;
    }

    return 0;
}

int SetVRAMPages(Item vram_io, void* dest, uint32 value, int32 pages, uint32 mask)
{
    if (!dest) return -1;

    // Simulate VRAM page setting
    uint32_t* pixels = (uint32_t*)dest;
    int pixels_per_page = g_graphics_base.gf_VRAMPageSize / 4; // 4 bytes per pixel
    int total_pixels = pages * pixels_per_page;

    uint32_t rgba_color = 0xFF000000 | value; // Add alpha

    for (int i = 0; i < total_pixels; i++) {
        if (mask == ~0L || (i & mask)) {
            pixels[i] = rgba_color;
        }
    }

    return 0;
}

int CopyVRAMPages(Item vram_io, void* dest, void* src, int32 pages, uint32 mask)
{
    if (!dest || !src) return -1;

    int bytes_to_copy = pages * g_graphics_base.gf_VRAMPageSize;
    
    if (mask == ~0L) {
        memcpy(dest, src, bytes_to_copy);
    } else {
        // Handle masked copy (simplified)
        uint32_t* dst = (uint32_t*)dest;
        uint32_t* src_pixels = (uint32_t*)src;
        int pixel_count = bytes_to_copy / 4;
        
        for (int i = 0; i < pixel_count; i++) {
            if (i & mask) {
                dst[i] = src_pixels[i];
            }
        }
    }

    return 0;
}

// VBL and timing
int GetVBLIOReq(void)
{
    return 1; // Return dummy VBL IO item
}

int WaitVBL(Item vbl_io, int32 frames)
{
    Uint32 current_time = SDL_GetTicks();
    Uint32 frame_duration = 1000 / VBL_RATE; // milliseconds per frame
    Uint32 target_time = g_last_vbl_time + (frames * frame_duration);

    if (current_time < target_time) {
        SDL_Delay(target_time - current_time);
    }

    g_last_vbl_time = SDL_GetTicks();
    return 0;
}

// Color functions
int SetScreenColor(Item screen_item, uint32 color_entry)
{
    // Extract color components from 3DO format
    int index = (color_entry >> 24) & 0xFF;
    int r = (color_entry >> 16) & 0xFF;
    int g = (color_entry >> 8) & 0xFF;
    int b = color_entry & 0xFF;

    // Store in color palette (simplified)
    printf("SetScreenColor: index=%d, r=%d, g=%d, b=%d\n", index, r, g, b);
    return 0;
}

// Memory management
void* AllocMem(int32 size, uint32 mem_flags)
{
    void* ptr = malloc(size);
    
    if (ptr && (mem_flags & MEMTYPE_FILL)) {
        memset(ptr, 0, size);
    }
    
    return ptr;
}

void FreeMem(void* ptr, int32 size)
{
    (void)size; // Unused in standard malloc/free
    free(ptr);
}

// Utility functions
void FasterMapCel(CCB* ccb, Point* corners)
{
    if (!ccb || !corners) return;

    // Calculate transformation matrix from corners
    // This is a simplified version - the original 3DO version was more complex
    ccb->ccb_XPos = Convert32_F16(corners[0].pt_X);
    ccb->ccb_YPos = Convert32_F16(corners[0].pt_Y);
    
    // Calculate deltas (simplified)
    ccb->ccb_HDX = Convert32_F16(corners[1].pt_X - corners[0].pt_X);
    ccb->ccb_HDY = Convert32_F16(corners[1].pt_Y - corners[0].pt_Y);
    ccb->ccb_VDX = Convert32_F16(corners[3].pt_X - corners[0].pt_X);
    ccb->ccb_VDY = Convert32_F16(corners[3].pt_Y - corners[0].pt_Y);
}

void SetFGPen(GrafCon* gc, Color color)
{
    if (gc) {
        gc->fg_color = color;
    }
}

void WritePixel(Item bitmap_item, GrafCon* gc, int32 x, int32 y)
{
    if (bitmap_item < 1 || bitmap_item > g_num_screens || !gc) {
        return;
    }

    Bitmap* bitmap = &g_bitmaps[bitmap_item - 1];
    if (x >= 0 && x < bitmap->bm_Width && y >= 0 && y < bitmap->bm_Height) {
        uint32_t* pixels = (uint32_t*)bitmap->bm_Buffer;
        int index = y * bitmap->bm_Width + x;
        pixels[index] = 0xFF000000 | gc->fg_color; // Add alpha
    }
}

// Accessor function for bitmaps (needed by game_stubs.c)
Bitmap* get_bitmap(Item bitmap_item) {
    if (bitmap_item < 1 || bitmap_item > g_num_screens) {
        return NULL;
    }
    return &g_bitmaps[bitmap_item - 1];
}

// Platform-specific functions needed by the main program
int clearscreen(RastPort* rp)
{
    if (!rp) return -1;
    
    // Clear with floor and ceiling colors (simplified)
    extern int32 floorcolor, ceilingcolor;
    
    if (!rp->rp_Bitmap || !rp->rp_Bitmap->bm_Buffer) {
        return -1;
    }

    uint32_t* pixels = (uint32_t*)rp->rp_Bitmap->bm_Buffer;
    int width = rp->rp_Bitmap->bm_Width;
    int height = rp->rp_Bitmap->bm_Height;
    
    // Split screen: ceiling on top, floor on bottom
    extern int32 cy; // Horizon line
    
    uint32_t ceiling_rgba = 0xFF000000 | ceilingcolor;
    uint32_t floor_rgba = 0xFF000000 | floorcolor;
    
    for (int y = 0; y < height; y++) {
        uint32_t color = (y < cy) ? ceiling_rgba : floor_rgba;
        for (int x = 0; x < width; x++) {
            pixels[y * width + x] = color;
        }
    }
    
    return 0;
}

void platform_clear_framebuffer(uint32 color)
{
    if (!g_renderer) return;
    
    // Extract RGB components from the color
    ubyte r = (color >> 16) & 0xFF;
    ubyte g = (color >> 8) & 0xFF;
    ubyte b = color & 0xFF;
    
    // Set the clear color and clear the screen
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void platform_present_framebuffer(void)
{
    if (!g_window) return;
    
    // Present the rendered frame to the screen
    if (g_use_opengl) {
        SDL_GL_SwapWindow(g_window);
    } else if (g_renderer) {
        SDL_RenderPresent(g_renderer);
    }
}

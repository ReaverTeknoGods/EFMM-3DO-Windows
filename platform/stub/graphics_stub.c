/*
 * graphics_stub.c - Stub graphics implementation for when SDL2 is not available
 * This allows the project to compile and demonstrate the porting approach
 */

#include "../../threedo_compat.h"
#include <stdio.h>

static int32 stub_screen_width = 320;
static int32 stub_screen_height = 240;
static int32 stub_initialized = 0;

int32 platform_init_graphics(int32 width, int32 height, int32 fullscreen) {
    printf("STUB: Initializing graphics %dx%d (fullscreen: %d)\n", width, height, fullscreen);
    stub_screen_width = width;
    stub_screen_height = height;
    stub_initialized = 1;
    return 0;
}

void platform_cleanup_graphics(void) {
    printf("STUB: Cleaning up graphics\n");
    stub_initialized = 0;
}

int32 platform_create_screen(RastPort* rp, int32 width, int32 height) {
    printf("STUB: Creating screen %dx%d\n", width, height);
    if (rp) {
        rp->rp_ScreenItem = 1;
        rp->rp_BitmapItem = 1;
        rp->rp_Bitmap = (Bitmap*)1;
        rp->rp_ScreenPtr = (void*)1;
    }
    return 0;
}

void platform_destroy_screen(RastPort* rp) {
    printf("STUB: Destroying screen\n");
    if (rp) {
        rp->rp_ScreenItem = 0;
        rp->rp_BitmapItem = 0;
        rp->rp_Bitmap = NULL;
        rp->rp_ScreenPtr = NULL;
    }
}

int32 DrawCels(Item bitmapItem, CCB* ccb) {
    printf("STUB: Drawing cel at (%d, %d)\n", 
           ccb ? ccb->ccb_XPos >> 16 : 0, 
           ccb ? ccb->ccb_YPos >> 16 : 0);
    return 0;
}

int32 DisplayScreen(Item screenItem, Item bitmapItem) {
    printf("STUB: Displaying screen\n");
    return 0;
}

int32 SetVRAMPages(Item ioreq, void* dest, void* src, int32 numpages, int32 mask) {
    printf("STUB: Setting VRAM pages\n");
    return 0;
}

int32 WaitVBL(Item ioreq, int32 numvbls) {
    printf("STUB: Waiting for %d VBL(s)\n", numvbls);
    // Simulate 60 FPS
    platform_sleep(16);
    return 0;
}

int32 SubmitVDL(void* vdlPtr, int32 length, int32 type) {
    printf("STUB: Submitting VDL\n");
    return 0;
}

int32 SetScreenColor(Item bitmapItem, uint32 color) {
    printf("STUB: Setting screen color to 0x%08X\n", color);
    return 0;
}

int32 SetScreenColors(Item bitmapItem, uint32* colors, int32 count) {
    printf("STUB: Setting %d screen colors\n", count);
    return 0;
}

void platform_swap_buffers(void) {
    printf("STUB: Swapping buffers\n");
}

void platform_clear_screen(uint32 color) {
    printf("STUB: Clearing screen with color 0x%08X\n", color);
}

void platform_set_viewport(int32 x, int32 y, int32 width, int32 height) {
    printf("STUB: Setting viewport to (%d, %d, %d, %d)\n", x, y, width, height);
}

int32 platform_get_screen_width(void) {
    return stub_screen_width;
}

int32 platform_get_screen_height(void) {
    return stub_screen_height;
}

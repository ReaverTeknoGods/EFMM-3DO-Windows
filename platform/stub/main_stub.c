/*
 * main_stub.c - Stub main implementation for when SDL2 is not available
 * This allows the project to compile and demonstrate the porting approach
 */

#include "../../threedo_compat.h"
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX
#define NOIME
#include <windows.h>
#undef CreateThread  // Avoid conflict with our CreateThread function
#else
#include <unistd.h>
#endif

static int32 stub_initialized = 0;
static uint32 stub_start_time = 0;

int32 platform_init(void) {
    printf("STUB: Initializing platform (stub mode)\n");
    printf("STUB: This is a demonstration build without SDL2\n");
    printf("STUB: Install SDL2 development libraries for full functionality\n");
    
    stub_start_time = (uint32)time(NULL);
    stub_initialized = 1;
    return 0;
}

void platform_cleanup(void) {
    printf("STUB: Cleaning up platform\n");
    stub_initialized = 0;
}

void platform_sleep(uint32 milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

uint32 platform_get_time(void) {
    if (!stub_initialized) return 0;
    return ((uint32)time(NULL) - stub_start_time) * 1000;
}

uint32 platform_get_ticks(void) {
    // Return milliseconds since initialization
    return platform_get_time();
}

void platform_pump_events(void) {
    // In stub mode, just update our simulated input
    platform_update_input();
}

int32 platform_main_loop_iteration(void) {
    // Return 1 to continue, 0 to quit
    static int32 iteration_count = 0;
    iteration_count++;
    
    platform_pump_events();
    
    // Run for a limited time in stub mode for demonstration
    if (iteration_count > 1000) {
        printf("STUB: Demonstration complete after %d iterations\n", iteration_count);
        return 0; // Signal to quit
    }
    
    return 1; // Continue running
}

void platform_set_window_title(const char* title) {
    printf("STUB: Setting window title: %s\n", title ? title : "NULL");
}

int32 platform_is_fullscreen(void) {
    return 0; // Always windowed in stub mode
}

void platform_toggle_fullscreen(void) {
    printf("STUB: Toggle fullscreen (not supported in stub mode)\n");
}

void platform_show_cursor(int32 show) {
    printf("STUB: %s cursor\n", show ? "Showing" : "Hiding");
}

void platform_warp_mouse(int32 x, int32 y) {
    printf("STUB: Warping mouse to (%d, %d)\n", x, y);
}

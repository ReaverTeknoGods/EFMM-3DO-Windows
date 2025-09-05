#include "platform/platform_types.h"
#include "platform/platform_graphics.h"
#include "platform/platform_audio.h"
#include "platform/platform_input.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * SDL2 main entry point
 * Replaces 3DO main initialization with cross-platform SDL2 setup
 */

// Forward declarations of original game functions
extern void parseargs(int ac, char **av);
extern void openstuff(void);
extern void closestuff(void);
extern int dotitle(void);
extern int dogame(void);

// Global variables referenced by original code
extern int8 skiptitle;
extern RastPort *rpvis, *rprend;

// Platform timing functions
// Note: platform_get_ticks() is implemented in game_stubs.c
// Uint32 platform_get_ticks(void)
// {
//     return SDL_GetTicks();
// }

void platform_delay(Uint32 ms)
{
    SDL_Delay(ms);
}

// Initialize platform systems
static int init_platform_systems(void)
{
    // Initialize graphics
    if (platform_init_graphics(320, 240, false) < 0) {
        printf("Failed to initialize graphics system\n");
        return -1;
    }

    // Initialize audio
    if (platform_init_audio() < 0) {
        printf("Failed to initialize audio system\n");
        platform_shutdown_graphics();
        return -1;
    }

    // Initialize input
    if (platform_init_input() < 0) {
        printf("Failed to initialize input system\n");
        platform_shutdown_audio();
        platform_shutdown_graphics();
        return -1;
    }

    return 0;
}

// Shutdown platform systems
static void shutdown_platform_systems(void)
{
    platform_shutdown_input();
    platform_shutdown_audio();
    platform_shutdown_graphics();
}

// Main game loop with modern event handling
static int run_game_loop(void)
{
    int running = 1;
    int game_result = 0;
    int in_title = 1;

    // Initialize game systems (original function)
    openstuff();

    // Title sequence
    if (!skiptitle) {
        printf("Starting title sequence loop...\n");
        int title_result = dotitle();
        if (title_result != 0 /* FC_NEWGAME equivalent */) {
            printf("Title sequence completed, starting main game...\n");
            in_title = 0;
        }
    } else {
        in_title = 0;
    }

    // Main game loop
    while (running) {
        // Handle SDL events
        if (platform_poll_events() != 0) {
            printf("Quit requested by user\n");
            running = 0; // Quit requested
            break;
        }

        // Update input state
        platform_update_input_state();

        if (in_title) {
            // Keep showing title screen and wait for input
            // Check for any key press to continue
            extern int joytrigger;
            if (joytrigger != 0) {
                printf("Key pressed, exiting title screen...\n");
                in_title = 0;
                continue;
            }
            
            // Small delay to prevent 100% CPU usage
            platform_delay(16); // ~60fps
            
        } else {
            // Run game logic (this would be modified to not have its own loop)
            game_result = dogame();
            
            // For now, exit after one game session
            // In a full implementation, dogame would be split into per-frame updates
            running = 0;
        }
    }

    // Cleanup game systems (original function)
    closestuff();
    
    return game_result;
}

// Cross-platform main function
int main(int argc, char* argv[])
{
    printf("Escape from Monster Manor - Cross-Platform Port\n");
    printf("Initializing...\n");

    // Initialize debug logging
    extern void init_debug_log(void);
    init_debug_log();

    // Parse command line arguments (original function)
    parseargs(argc, argv);

    // Initialize platform systems
    if (init_platform_systems() < 0) {
        return 1;
    }

    // Run the game
    int result = run_game_loop();

    // Shutdown platform systems
    shutdown_platform_systems();

    // Close debug logging
    extern void close_debug_log(void);
    close_debug_log();

    printf("Game exited with code %d\n", result);
    return result;
}

// Additional utility functions for the port

// File system abstraction
int platform_file_exists(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// Note: platform_get_resource_path() is implemented in game_stubs.c
/*
char* platform_get_resource_path(const char* filename)
{
    static char path[512];
    
    // Try different possible locations for game assets
    const char* search_paths[] = {
        "./",
        "./assets/",
        "./data/",
        "../assets/",
        "../data/",
        NULL
    };

    for (int i = 0; search_paths[i]; i++) {
        snprintf(path, sizeof(path), "%s%s", search_paths[i], filename);
        if (platform_file_exists(path)) {
            return path;
        }
    }

    // If not found, return original filename
    strncpy(path, filename, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    return path;
}
*/

// Memory debugging helpers
#ifdef DEBUG
static size_t g_total_allocated = 0;
static int g_allocation_count = 0;

void* debug_malloc(size_t size, const char* file, int line)
{
    void* ptr = malloc(size + sizeof(size_t));
    if (ptr) {
        *(size_t*)ptr = size;
        g_total_allocated += size;
        g_allocation_count++;
        printf("MALLOC: %zu bytes at %p (%s:%d) [Total: %zu, Count: %d]\n", 
               size, (char*)ptr + sizeof(size_t), file, line, g_total_allocated, g_allocation_count);
        return (char*)ptr + sizeof(size_t);
    }
    return NULL;
}

void debug_free(void* ptr, const char* file, int line)
{
    if (ptr) {
        void* real_ptr = (char*)ptr - sizeof(size_t);
        size_t size = *(size_t*)real_ptr;
        g_total_allocated -= size;
        g_allocation_count--;
        printf("FREE: %zu bytes at %p (%s:%d) [Total: %zu, Count: %d]\n", 
               size, ptr, file, line, g_total_allocated, g_allocation_count);
        free(real_ptr);
    }
}

#define malloc(size) debug_malloc(size, __FILE__, __LINE__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__)
#endif

// Note: platform_error() is implemented in game_stubs.c
/*
// Error handling
void platform_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    printf("ERROR: ");
    vprintf(format, args);
    printf("\n");
    
    va_end(args);
    
    // Show error dialog on desktop platforms
#ifdef _WIN32
    // Windows MessageBox implementation would go here
#elif defined(__APPLE__)
    // macOS NSAlert implementation would go here  
#else
    // Linux notification implementation would go here
#endif
}
*/

// Performance monitoring
typedef struct PerformanceCounter {
    Uint32 start_time;
    Uint32 total_time;
    int call_count;
    const char* name;
} PerformanceCounter;

static PerformanceCounter g_perf_counters[16];
static int g_num_perf_counters = 0;

void platform_perf_start(const char* name)
{
    // Find or create performance counter
    int index = -1;
    for (int i = 0; i < g_num_perf_counters; i++) {
        if (strcmp(g_perf_counters[i].name, name) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1 && g_num_perf_counters < 16) {
        index = g_num_perf_counters++;
        g_perf_counters[index].name = name;
        g_perf_counters[index].total_time = 0;
        g_perf_counters[index].call_count = 0;
    }
    
    if (index >= 0) {
        g_perf_counters[index].start_time = SDL_GetTicks();
    }
}

void platform_perf_end(const char* name)
{
    Uint32 end_time = SDL_GetTicks();
    
    for (int i = 0; i < g_num_perf_counters; i++) {
        if (strcmp(g_perf_counters[i].name, name) == 0) {
            g_perf_counters[i].total_time += end_time - g_perf_counters[i].start_time;
            g_perf_counters[i].call_count++;
            break;
        }
    }
}

void platform_perf_report(void)
{
    printf("\n=== Performance Report ===\n");
    for (int i = 0; i < g_num_perf_counters; i++) {
        PerformanceCounter* counter = &g_perf_counters[i];
        if (counter->call_count > 0) {
            float avg_time = (float)counter->total_time / counter->call_count;
            printf("%s: %d calls, %.2f ms average, %.2f ms total\n",
                   counter->name, counter->call_count, avg_time, (float)counter->total_time);
        }
    }
    printf("===========================\n\n");
}

// Configuration management
typedef struct GameConfig {
    int screen_width;
    int screen_height;
    bool fullscreen;
    bool vsync;
    float master_volume;
    float music_volume;
    float sfx_volume;
    int difficulty;
} GameConfig;

static GameConfig g_game_config = {
    .screen_width = 640,
    .screen_height = 480,
    .fullscreen = false,
    .vsync = true,
    .master_volume = 1.0f,
    .music_volume = 0.8f,
    .sfx_volume = 1.0f,
    .difficulty = 1
};

void platform_load_config(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Config file not found, using defaults\n");
        return;
    }

    // Simple key=value parser
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char key[64], value[64];
        if (sscanf(line, "%63[^=]=%63s", key, value) == 2) {
            if (strcmp(key, "screen_width") == 0) {
                g_game_config.screen_width = atoi(value);
            } else if (strcmp(key, "screen_height") == 0) {
                g_game_config.screen_height = atoi(value);
            } else if (strcmp(key, "fullscreen") == 0) {
                g_game_config.fullscreen = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "master_volume") == 0) {
                g_game_config.master_volume = atof(value);
            }
            // Add more config options as needed
        }
    }

    fclose(file);
    printf("Loaded configuration from %s\n", filename);
}

void platform_save_config(const char* filename)
{
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to save config to %s\n", filename);
        return;
    }

    fprintf(file, "screen_width=%d\n", g_game_config.screen_width);
    fprintf(file, "screen_height=%d\n", g_game_config.screen_height);
    fprintf(file, "fullscreen=%s\n", g_game_config.fullscreen ? "true" : "false");
    fprintf(file, "master_volume=%.2f\n", g_game_config.master_volume);
    fprintf(file, "music_volume=%.2f\n", g_game_config.music_volume);
    fprintf(file, "sfx_volume=%.2f\n", g_game_config.sfx_volume);
    fprintf(file, "difficulty=%d\n", g_game_config.difficulty);

    fclose(file);
    printf("Saved configuration to %s\n", filename);
}

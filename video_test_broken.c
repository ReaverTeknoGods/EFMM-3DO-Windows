#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include "cinepak_decode.h"

// SDL2 includes for video display
#ifdef _WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

// Debug printf function for CinePak decoder
void debug_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// 3DO Data Stream structures
typedef struct {
    uint32_t chunk_type;    // 4-byte chunk identifier
    uint32_t chunk_size;    // Total size including header
    uint32_t chunk_time;    // Time in audio folio ticks
    uint32_t chunk_channel; // Channel number
} StreamChunkHeader;

// Global SDL variables
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* texture = NULL;
uint32_t* frame_buffer = NULL;

// Initialize SDL graphics
bool init_graphics() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed: %s\n", SDL_GetError());
        return false;
    }
    
    window = SDL_CreateWindow("3DO Video Test", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, 
        SDL_TEXTUREACCESS_STREAMING, 320, 240);
    if (!texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    frame_buffer = (uint32_t*)malloc(320 * 240 * sizeof(uint32_t));
    if (!frame_buffer) {
        printf("Frame buffer allocation failed\n");
        return false;
    }
    
    // Clear to black
    memset(frame_buffer, 0, 320 * 240 * sizeof(uint32_t));
    
    printf("Graphics initialized: 320x240 (window: 640x480)\n");
    return true;
}

// Display frame buffer to screen
void display_frame() {
    SDL_UpdateTexture(texture, NULL, frame_buffer, 320 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

// Clean up graphics
void cleanup_graphics() {
    if (frame_buffer) free(frame_buffer);
    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

// Process SDL events
bool process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return false;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            return false;
        }
    }
    return true;
}

// Convert RGB24 to RGBA32 and center in frame buffer
void convert_and_display(unsigned char* rgb_buffer, int width, int height) {
    printf("  Converting %dx%d video to display\n", width, height);
    
    // Clear frame buffer to dark grey to see if conversion is working
    for (int i = 0; i < 320 * 240; i++) {
        frame_buffer[i] = 0xFF404040; // Dark grey background
    }
    
    // Calculate centering offsets
    int offset_x = (320 - width) / 2;
    int offset_y = (240 - height) / 2;
    printf("  Centering offset: (%d, %d)\n", offset_x, offset_y);
    
    // Sample a few pixels to check RGB values
    bool sampled = false;
    
    // Convert RGB24 to RGBA32 and center
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int src_idx = (y * width + x) * 3;
            int dst_x = offset_x + x;
            int dst_y = offset_y + y;
            
            if (dst_x >= 0 && dst_x < 320 && dst_y >= 0 && dst_y < 240) {
                uint8_t r = rgb_buffer[src_idx + 0];
                uint8_t g = rgb_buffer[src_idx + 1];
                uint8_t b = rgb_buffer[src_idx + 2];
                
                // Sample a few pixels for debugging
                if (!sampled && x < 5 && y < 5) {
                    printf("    Pixel [%d,%d]: RGB(%d,%d,%d)\n", x, y, r, g, b);
                    if (x == 4 && y == 4) sampled = true;
                }
                
                uint32_t rgba = 0xFF000000 | (r << 16) | (g << 8) | b;
                frame_buffer[dst_y * 320 + dst_x] = rgba;
            }
        }
    }
    
    display_frame();
}

// Play 3DO video file
int play_video(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open video file: %s\n", filename);
        return -1;
    }
    
    printf("Playing 3DO video: %s\n", filename);
    
    // Initialize CinePak decoder
    void* cinepak_context = decode_cinepak_init();
    if (!cinepak_context) {
        printf("Failed to initialize CinePak decoder\n");
        fclose(file);
        return -1;
    }
    
    StreamChunkHeader header;
    uint32_t film_frame_count = 0;
    uint32_t video_width = 0, video_height = 0;
    bool dimensions_detected = false;
    
    // For hold frame functionality
    uint32_t* previous_frame = (uint32_t*)malloc(320 * 240 * sizeof(uint32_t));
    bool has_previous_frame = false;
    
    printf("Parsing 3DO Data Stream chunks...\n");
    
    int chunk_count = 0;
    while (fread(&header, sizeof(StreamChunkHeader), 1, file) == 1) {
        chunk_count++;
        
        // Check for reasonable chunk header values before conversion
        if (chunk_count == 1) {
            printf("Raw first chunk header: %08X %08X %08X %08X\n", 
                   header.chunk_type, header.chunk_size, header.chunk_time, header.chunk_channel);
        }
        
        // Convert from big-endian
        header.chunk_type = (header.chunk_type << 24) | ((header.chunk_type << 8) & 0xFF0000) |
                           ((header.chunk_type >> 8) & 0xFF00) | (header.chunk_type >> 24);
        header.chunk_size = (header.chunk_size << 24) | ((header.chunk_size << 8) & 0xFF0000) |
                           ((header.chunk_size >> 8) & 0xFF00) | (header.chunk_size >> 24);
        header.chunk_time = (header.chunk_time << 24) | ((header.chunk_time << 8) & 0xFF0000) |
                           ((header.chunk_time >> 8) & 0xFF00) | (header.chunk_time >> 24);
        header.chunk_channel = (header.chunk_channel << 24) | ((header.chunk_channel << 8) & 0xFF0000) |
                              ((header.chunk_channel >> 8) & 0xFF00) | (header.chunk_channel >> 24);
        
        // Sanity check the converted values
        if (header.chunk_size < sizeof(StreamChunkHeader) || header.chunk_size > 1000000) {
            printf("ERROR: Invalid chunk size %u (0x%08X) at chunk %d\n", 
                   header.chunk_size, header.chunk_size, chunk_count);
            printf("This suggests file corruption or wrong endian conversion\n");
            break;
        }
        
        uint32_t data_size = header.chunk_size - sizeof(StreamChunkHeader);
        
        // Convert chunk type to string
        char chunk_name[5];
        chunk_name[0] = (header.chunk_type >> 24) & 0xFF;
        chunk_name[1] = (header.chunk_type >> 16) & 0xFF;
        chunk_name[2] = (header.chunk_type >> 8) & 0xFF;
        chunk_name[3] = header.chunk_type & 0xFF;
        chunk_name[4] = '\0';
        
        // Check for printable characters in chunk name
        bool valid_chunk_name = true;
        for (int i = 0; i < 4; i++) {
            if (chunk_name[i] < 32 || chunk_name[i] > 126) {
                valid_chunk_name = false;
                break;
            }
        }
        
        if (!valid_chunk_name) {
            printf("ERROR: Invalid chunk name at chunk %d (bytes: %02X %02X %02X %02X)\n", 
                   chunk_count, 
                   (header.chunk_type >> 24) & 0xFF,
                   (header.chunk_type >> 16) & 0xFF, 
                   (header.chunk_type >> 8) & 0xFF,
                   header.chunk_type & 0xFF);
            break;
        }
        
        printf("Chunk %d: %s, size: %u, time: %u, data: %u bytes\n", 
               chunk_count, chunk_name, header.chunk_size, header.chunk_time, data_size);
        
        if (header.chunk_type == 0x46494C4D) { // "FILM" - CinePak video frame
            film_frame_count++;
            printf("  -> Video frame #%u\n", film_frame_count);
            
            if (data_size >= 20) {
                // Read frame header to detect dimensions and type
                unsigned char frame_header[64];
                int bytes_to_read = (data_size >= 64) ? 64 : 20;
                if (fread(frame_header, 1, bytes_to_read, file) == bytes_to_read) {
                    
                    // Check frame signature
                    uint32_t frame_sig = (frame_header[0] << 24) | (frame_header[1] << 16) | 
                                        (frame_header[2] << 8) | frame_header[3];
                    
                    char sig_str[5];
                    sig_str[0] = (frame_sig >> 24) & 0xFF;
                    sig_str[1] = (frame_sig >> 16) & 0xFF;
                    sig_str[2] = (frame_sig >> 8) & 0xFF;
                    sig_str[3] = frame_sig & 0xFF;
                    sig_str[4] = '\0';
                    
                    printf("  Frame signature: %s (0x%08X)\n", sig_str, frame_sig);
                    
                    // Only process FRME frames, skip FHDR frames
                    if (frame_sig == 0x46524D45) { // "FRME"
                        printf("  -> FRME frame (contains video data)\n");
                        
                        // Extract dimensions from bytes 16-19
                        if (bytes_to_read >= 20) {
                            uint16_t width = (frame_header[16] << 8) | frame_header[17];
                            uint16_t height = (frame_header[18] << 8) | frame_header[19];
                            
                            if (width > 0 && height > 0 && width <= 640 && height <= 480) {
                                if (!dimensions_detected || video_width != width || video_height != height) {
                                    video_width = width;
                                    video_height = height;
                                    dimensions_detected = true;
                                    printf("  Video dimensions: %ux%u\n", video_width, video_height);
                                }
                            }
                        }
                        
                        // Reset file position and read entire frame data
                        fseek(file, -(long)bytes_to_read, SEEK_CUR);
                        
                        if (dimensions_detected && 
                            ((video_width == 280 && video_height == 200) || 
                             (video_width == 320 && video_height == 240))) {
                            
                            printf("  -> Decoding CinePak frame %u (%ux%u)\n", film_frame_count, video_width, video_height);
                            
                            // Allocate buffers
                            unsigned char* frame_data = (unsigned char*)malloc(data_size);
                            unsigned char* rgb_buffer = (unsigned char*)malloc(video_width * video_height * 3);
                            
                            if (frame_data && rgb_buffer) {
                                if (fread(frame_data, 1, data_size, file) == data_size) {
                                    // Skip 40-byte FRME header to get to potential CinePak data
                                    unsigned char* cinepak_data = frame_data + 40;
                                    uint32_t cinepak_size = data_size - 40;
                                    
                                    // Check frame type (3DO custom format)
                                    uint8_t frame_type = cinepak_data[0];
                                    printf("  Frame type: 0x%02X\n", frame_type);
                                    
                                    if (frame_type == 0x21) {
                                        // 0x21 frames are "hold frames" - repeat previous frame
                                        printf("  -> 0x21 frame detected: Hold frame (repeat previous)\n");
                                        
                                        // Simply redisplay the current frame buffer without decoding
                                        display_frame();
                                        printf("  -> Hold frame displayed successfully\n");
                                    } else if (frame_type == 0x20) {
                                        // 0x20 frames contain actual CinePak data
                                        printf("  -> 0x20 frame detected: CinePak keyframe\n");
                                        
                                        // Validate CinePak data size
                                        if (cinepak_size < 10) {
                                            printf("  -> CinePak data too small (%u bytes), skipping\n", cinepak_size);
                                            goto skip_frame;
                                        }
                                        
                                        // 3DO CinePak has non-standard headers
                                        // Try to decode by skipping broken header and using FHDR dimensions
                                        printf("        Attempting 3DO CinePak decode with manual dimensions\n");
                                        
                                        int result = 1; // Default to error
                                        
                                        // Clear the RGB buffer
                                        memset(rgb_buffer, 0, video_width * video_height * 3);
                                        
                                        // Skip the first 10 bytes which contain the broken header
                                        // Standard CinePak starts with: flags(1) + len(3) + width(2) + height(2) + strips(2)
                                        // But 3DO format has these fields corrupted/different
                                        
                                        if (cinepak_size > 20) {
                                            // Try to find actual CinePak strip data by scanning for patterns
                                            unsigned char* strip_data = NULL;
                                            uint32_t strip_size = 0;
                                            
                                            // Look for potential strip headers in the data
                                            printf("        CinePak raw data (first 64 bytes): ");
                                            for (int i = 0; i < 64 && i < cinepak_size; i++) {
                                                printf("%02X ", cinepak_data[i]);
                                                if ((i + 1) % 16 == 0) printf("\n                                                      ");
                                            }
                                            printf("\n");
                                            
                                            // 3DO CinePak format is completely custom - build a proper CinePak frame manually
                                            printf("        Building custom CinePak frame from 3DO data\n");
                                            
                                            // Create a complete CinePak frame from scratch
                                            size_t total_frame_size = 20 + cinepak_size;  // Header + strip header + data
                                            unsigned char* full_cinepak_frame = malloc(total_frame_size);
                                            if (!full_cinepak_frame) {
                                                printf("        Failed to allocate memory for CinePak frame\n");
                                                return;
                                            }
                                            
                                            size_t pos = 0;
                                            
                                            // CinePak file header (10 bytes)
                                            full_cinepak_frame[pos++] = 0x00;  // frame_flags
                                            full_cinepak_frame[pos++] = (total_frame_size >> 16) & 0xFF;  // length[2]
                                            full_cinepak_frame[pos++] = (total_frame_size >> 8) & 0xFF;   // length[1] 
                                            full_cinepak_frame[pos++] = total_frame_size & 0xFF;          // length[0]
                                            full_cinepak_frame[pos++] = (video_width >> 8) & 0xFF;        // width[1]
                                            full_cinepak_frame[pos++] = video_width & 0xFF;               // width[0]
                                            full_cinepak_frame[pos++] = (video_height >> 8) & 0xFF;       // height[1]
                                            full_cinepak_frame[pos++] = video_height & 0xFF;              // height[0]
                                            full_cinepak_frame[pos++] = 0x00;  // strips[1]
                                            full_cinepak_frame[pos++] = 0x01;  // strips[0] - assume 1 strip
                                            
                                            // CinePak strip header (12 bytes)
                                            full_cinepak_frame[pos++] = 0x10;  // strip_id[1] (0x1000 = key strip)
                                            full_cinepak_frame[pos++] = 0x00;  // strip_id[0]
                                            size_t strip_size_field = 12 + cinepak_size;
                                            full_cinepak_frame[pos++] = (strip_size_field >> 8) & 0xFF;   // strip_size[1]
                                            full_cinepak_frame[pos++] = strip_size_field & 0xFF;          // strip_size[0]
                                            full_cinepak_frame[pos++] = 0x00;  // y0[1]
                                            full_cinepak_frame[pos++] = 0x00;  // y0[0]
                                            full_cinepak_frame[pos++] = 0x00;  // x0[1] 
                                            full_cinepak_frame[pos++] = 0x00;  // x0[0]
                                            full_cinepak_frame[pos++] = (video_height >> 8) & 0xFF;       // y1[1]
                                            full_cinepak_frame[pos++] = video_height & 0xFF;              // y1[0]
                                            full_cinepak_frame[pos++] = (video_width >> 8) & 0xFF;        // x1[1]
                                            full_cinepak_frame[pos++] = video_width & 0xFF;               // x1[0]
                                            
                                            // Copy the 3DO CinePak data (skip the first 4 bytes which are frame type/size)
                                            memcpy(full_cinepak_frame + pos, cinepak_data + 4, cinepak_size - 4);
                                            
                                            printf("        Created %zu byte CinePak frame with %zu bytes of 3DO data\n", 
                                                   total_frame_size, cinepak_size - 4);
                                            
                                            strip_data = full_cinepak_frame;
                                            strip_size = total_frame_size;
                                            
                                            printf("        Trying CinePak decode with custom 3DO frame...\n");
                                            
                                            if (cinepak_decode_frame(video_width, video_height, strip_data, strip_size, 
                                                                   frame_buffer, video_width * 4) == 0) {
                                                printf("        3DO CinePak decode successful!\n");
                                                // Update texture with decoded frame
                                                SDL_UpdateTexture(texture, NULL, frame_buffer, video_width * 4);
                                                
                                                // Store the decoded frame as previous frame for hold frames
                                                memcpy(previous_frame, frame_buffer, video_width * video_height * 4);
                                                has_previous_frame = 1;
                                            } else {
                                                printf("        3DO CinePak decode failed\n");
                                                // Show error pattern
                                                for (int i = 0; i < video_width * video_height; i++) {
                                                    ((uint32_t*)frame_buffer)[i] = 0xFF808080;  // Gray
                                                }
                                                SDL_UpdateTexture(texture, NULL, frame_buffer, video_width * 4);
                                            }
                                            
                                            // Free the allocated frame
                                            free(full_cinepak_frame);
                                        } else if (frame_type == 0x21) {
                                            printf("  -> 0x21 frame detected: Hold frame (repeat previous)\n");
                                            
                                            if (has_previous_frame) {
                                                // Copy previous frame to current frame buffer
                                                memcpy(frame_buffer, previous_frame, video_width * video_height * 4);
                                                SDL_UpdateTexture(texture, NULL, frame_buffer, video_width * 4);
                                                printf("  -> Hold frame displayed successfully\n");
                                            } else {
                                                printf("  -> No previous frame to hold, showing test pattern\n");
                                                // Show test pattern
                                                for (int i = 0; i < video_width * video_height; i++) {
                                                    ((uint32_t*)frame_buffer)[i] = 0xFFFF0000;  // Red
                                                }
                                                SDL_UpdateTexture(texture, NULL, frame_buffer, video_width * 4);
                                            }
                                        } else {
                                            printf("  -> Unknown frame type 0x%02X, skipping\n", frame_type);
                                        }
                                    } else {
                                        printf("  Failed to read frame data\n");
                                    }
                                    
                                    if (frame_data) free(frame_data);
                                } else {
                            printf("  -> Skipping frame (unsupported dimensions or not detected yet)\n");
                            fseek(file, data_size, SEEK_CUR);
                        }
                    } else if (frame_sig == 0x46484452) { // "FHDR"
                        printf("  -> FHDR frame (header only, skipping)\n");
                        // Skip remaining data after the header we already read
                        fseek(file, data_size - bytes_to_read, SEEK_CUR);
                    } else {
                        printf("  -> Unknown frame type, skipping\n");
                        // Skip the entire chunk data
                        fseek(file, data_size - bytes_to_read, SEEK_CUR);
                    }
                } else {
                    printf("  Failed to read frame header\n");
                    fseek(file, data_size, SEEK_CUR);
                }
            } else {
                printf("  Frame too small, skipping\n");
                fseek(file, data_size, SEEK_CUR);
            }
            
            // Process events and add small delay
            if (!process_events()) {
                printf("User requested exit\n");
                break;
            }
            
            SDL_Delay(100); // ~10fps for testing
            
        } else {
            // Skip non-video chunks
            fseek(file, data_size, SEEK_CUR);
        }
        
        // Safety limit to prevent infinite loops
        if (chunk_count > 1000) {
            printf("Reached chunk limit (%d), stopping to prevent infinite loop\n", chunk_count);
            break;
        }
        
        // Safety limit - increased since we now handle 0x21 frames properly
        if (film_frame_count > 20) {
            printf("Reached frame limit (20), stopping for debugging\n");
            break;
        }
    }
    
    printf("Video playback complete. Processed %u frames.\n", film_frame_count);
    printf("Final dimensions: %ux%u\n", video_width, video_height);
    
    decode_cinepak_free(cinepak_context);
    fclose(file);
    return 0;
}

int SDL_main(int argc, char* argv[]) {
    printf("3DO Video Test Player\n");
    printf("====================\n");
    
    if (argc != 2) {
        printf("Usage: %s <video_file>\n", argv[0]);
        printf("Example: %s PublisherLogo.stream\n", argv[0]);
        return 1;
    }
    
    if (!init_graphics()) {
        printf("Failed to initialize graphics\n");
        return 1;
    }
    
    printf("Controls: ESC or close window to exit\n\n");
    
    int result = play_video(argv[1]);
    
    printf("\nPress ESC or close window to exit...\n");
    while (process_events()) {
        SDL_Delay(100);
    }
    
    cleanup_graphics();
    return result;
}

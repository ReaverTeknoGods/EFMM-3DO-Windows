#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <SDL.h>

// 3DO Data Stream chunk header
typedef struct {
    uint32_t chunk_type;
    uint32_t chunk_size;
    uint32_t chunk_time;
    uint32_t chunk_channel;
} StreamChunkHeader;

// External CinePak decoder functions
extern void* decode_cinepak_init(void);
extern void decode_cinepak_free(void* context);
extern int decode_cinepak(void *context, unsigned char *buf, int size, 
                         unsigned char *frame, int width, int height, int bit_per_pixel);

// Debug function for CinePak decoder
void debug_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Graphics variables
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static uint32_t* frame_buffer = NULL;

// Initialize graphics
bool init_graphics() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
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

// Play 3DO video file
int play_video(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open video file: %s\n", filename);
        return -1;
    }
    
    printf("Playing 3DO video: %s\n", filename);
    printf("Parsing 3DO Data Stream chunks...\n");
    
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
            printf("ERROR: Invalid chunk name at chunk %d\n", chunk_count);
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
                    
                    if (frame_sig == 0x46524D45) { // "FRME"
                        printf("  Frame signature: FRME (0x%08X)\n", frame_sig);
                        printf("  -> FRME frame (contains video data)\n");
                        
                        // Extract dimensions from frame header (3DO format)
                        if (!dimensions_detected) {
                            // Try to extract dimensions from FHDR data we've seen
                            video_width = 280; 
                            video_height = 200;
                            dimensions_detected = true;
                            printf("  Video dimensions: %ux%u\n", video_width, video_height);
                        }
                        
                        // Reset file position and read entire frame data
                        fseek(file, -(long)bytes_to_read, SEEK_CUR);
                        
                        if (dimensions_detected) {
                            printf("  -> Decoding CinePak frame %u (%ux%u)\n", film_frame_count, video_width, video_height);
                            
                            // Allocate frame data buffer
                            unsigned char* frame_data = (unsigned char*)malloc(data_size);
                            
                            if (frame_data) {
                                if (fread(frame_data, 1, data_size, file) == data_size) {
                                    // Skip 40-byte FRME header to get to potential CinePak data
                                    unsigned char* cinepak_data = frame_data + 40;
                                    uint32_t cinepak_size = data_size - 40;
                                    
                                    // Check frame type (3DO custom format)
                                    if (cinepak_size >= 4) {
                                        uint8_t frame_type = cinepak_data[0];
                                        printf("  Frame type: 0x%02X\n", frame_type);
                                        
                                        if (frame_type == 0x20) {
                                            printf("  -> 0x20 frame detected: CinePak keyframe\n");
                                            printf("        Attempting 3DO CinePak decode with manual dimensions\n");
                                            
                                            // 3DO CinePak format is completely custom - build a proper CinePak frame manually
                                            printf("        Building custom CinePak frame from 3DO data\n");
                                            
                                            // Create a complete CinePak frame from scratch
                                            size_t total_frame_size = 22 + cinepak_size;  // Header + strip header + data
                                            unsigned char* full_cinepak_frame = malloc(total_frame_size);
                                            if (full_cinepak_frame) {
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
                                                size_t strip_size_field = 12 + (cinepak_size - 4);  // 12 byte header + actual data size
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
                                                
                                                printf("        Trying CinePak decode with custom 3DO frame...\n");
                                                
                                                // Allocate RGB buffer for the decoder output  
                                                unsigned char* rgb_buffer = malloc(video_width * video_height * 3);
                                                if (rgb_buffer) {
                                                    if (decode_cinepak(cinepak_context, full_cinepak_frame, total_frame_size, 
                                                                      rgb_buffer, video_width, video_height, 24) == 0) {
                                                        printf("        3DO CinePak decode successful!\n");
                                                        
                                                        // Sample first few pixels to see what we got
                                                        printf("        Sample RGB values: ");
                                                        for (int i = 0; i < 15 && i < video_width * video_height; i++) {
                                                            printf("(%d,%d,%d) ", rgb_buffer[i*3], rgb_buffer[i*3+1], rgb_buffer[i*3+2]);
                                                        }
                                                        printf("\n");
                                                        
                                                        // Check if all pixels are black
                                                        bool all_black = true;
                                                        for (int i = 0; i < video_width * video_height * 3; i++) {
                                                            if (rgb_buffer[i] != 0) {
                                                                all_black = false;
                                                                break;
                                                            }
                                                        }
                                                        
                                                        if (all_black) {
                                                            printf("        WARNING: All pixels are black! CinePak decoder may have failed silently.\n");
                                                            // Show red test pattern instead
                                                            for (int i = 0; i < 320 * 240; i++) {
                                                                frame_buffer[i] = 0xFFFF0000; // Red
                                                            }
                                                        } else {
                                                            printf("        Found non-black pixels, converting to display format\n");
                                                            // Convert RGB24 to RGBA32 and center in frame buffer
                                                            for (int i = 0; i < 320 * 240; i++) {
                                                                frame_buffer[i] = 0xFF404040; // Dark grey background
                                                            }
                                                            
                                                            // Calculate centering offsets
                                                            int offset_x = (320 - video_width) / 2;
                                                            int offset_y = (240 - video_height) / 2;
                                                            
                                                            // Convert RGB24 to RGBA32 and center
                                                            for (int y = 0; y < video_height; y++) {
                                                                for (int x = 0; x < video_width; x++) {
                                                                    int src_idx = (y * video_width + x) * 3;
                                                                    int dst_x = offset_x + x;
                                                                    int dst_y = offset_y + y;
                                                                    
                                                                    if (dst_x >= 0 && dst_x < 320 && dst_y >= 0 && dst_y < 240) {
                                                                        uint8_t r = rgb_buffer[src_idx + 0];
                                                                        uint8_t g = rgb_buffer[src_idx + 1];
                                                                        uint8_t b = rgb_buffer[src_idx + 2];
                                                                        
                                                                        uint32_t rgba = 0xFF000000 | (r << 16) | (g << 8) | b;
                                                                        frame_buffer[dst_y * 320 + dst_x] = rgba;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                        
                                                        SDL_UpdateTexture(texture, NULL, frame_buffer, 320 * 4);
                                                        
                                                        // Store the decoded frame as previous frame for hold frames
                                                        memcpy(previous_frame, frame_buffer, 320 * 240 * 4);
                                                        has_previous_frame = 1;
                                                    } else {
                                                        printf("        3DO CinePak decode failed\n");
                                                        // Show error pattern
                                                        for (int i = 0; i < 320 * 240; i++) {
                                                            frame_buffer[i] = 0xFF808080;  // Gray
                                                        }
                                                        SDL_UpdateTexture(texture, NULL, frame_buffer, 320 * 4);
                                                    }
                                                    
                                                    free(rgb_buffer);
                                                } else {
                                                    printf("        Failed to allocate RGB buffer\n");
                                                }
                                                
                                                // Free the allocated frame
                                                free(full_cinepak_frame);
                                            } else {
                                                printf("        Failed to allocate CinePak frame memory\n");
                                            }
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
                                        printf("  -> Insufficient CinePak data\n");
                                    }
                                } else {
                                    printf("  Failed to read frame data\n");
                                }
                                
                                free(frame_data);
                            } else {
                                printf("  Failed to allocate frame data buffer\n");
                                fseek(file, data_size, SEEK_CUR);
                            }
                        } else {
                            printf("  -> Skipping frame (dimensions not detected yet)\n");
                            fseek(file, data_size - bytes_to_read, SEEK_CUR);
                        }
                    } else if (frame_sig == 0x46484452) { // "FHDR"
                        printf("  Frame signature: FHDR (0x%08X)\n", frame_sig);
                        printf("  -> FHDR frame (header only, skipping)\n");
                        // Skip remaining data after the header we already read
                        fseek(file, data_size - bytes_to_read, SEEK_CUR);
                    } else {
                        printf("  -> Unknown frame signature 0x%08X, skipping\n", frame_sig);
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
    if (previous_frame) free(previous_frame);
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

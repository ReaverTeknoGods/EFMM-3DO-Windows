#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    uint32_t chunk_type;
    uint32_t chunk_size;
    uint32_t chunk_time;
    uint32_t channel;
} ChunkHeader;

void print_hex_dump(const unsigned char* data, int size, const char* label) {
    printf("\n%s (%d bytes):\n", label, size);
    for (int i = 0; i < size; i += 16) {
        printf("%04X: ", i);
        // Hex bytes
        for (int j = 0; j < 16; j++) {
            if (i + j < size) {
                printf("%02X ", data[i + j]);
            } else {
                printf("   ");
            }
        }
        printf(" ");
        // ASCII representation
        for (int j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = data[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");
    }
}

uint32_t read_be32(FILE* file) {
    unsigned char buf[4];
    if (fread(buf, 1, 4, file) != 4) return 0;
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

void analyze_film_chunk(FILE* file, uint32_t data_size) {
    unsigned char* frame_data = malloc(data_size);
    if (!frame_data) return;
    
    if (fread(frame_data, 1, data_size, file) != data_size) {
        free(frame_data);
        return;
    }
    
    // Check frame signature
    uint32_t frame_sig = (frame_data[0] << 24) | (frame_data[1] << 16) | 
                        (frame_data[2] << 8) | frame_data[3];
    
    char sig_str[5];
    sig_str[0] = (frame_sig >> 24) & 0xFF;
    sig_str[1] = (frame_sig >> 16) & 0xFF;
    sig_str[2] = (frame_sig >> 8) & 0xFF;
    sig_str[3] = frame_sig & 0xFF;
    sig_str[4] = '\0';
    
    printf("  Frame signature: %s (0x%08X)\n", sig_str, frame_sig);
    
    if (frame_sig == 0x46524D45) { // "FRME"
        printf("  -> FRME frame analysis:\n");
        
        // Print FRME header (first 40 bytes)
        print_hex_dump(frame_data, data_size < 64 ? data_size : 64, "FRME Header");
        
        // Extract dimensions from bytes 16-19
        if (data_size >= 20) {
            uint16_t width = (frame_data[16] << 8) | frame_data[17];
            uint16_t height = (frame_data[18] << 8) | frame_data[19];
            printf("  Dimensions from header: %ux%u\n", width, height);
        }
        
        // Analyze potential CinePak data after 40-byte header
        if (data_size > 40) {
            unsigned char* cinepak_area = frame_data + 40;
            uint32_t cinepak_size = data_size - 40;
            
            printf("  Potential CinePak data (after 40-byte header):\n");
            uint8_t frame_type = cinepak_area[0];
            printf("    First byte (frame type): 0x%02X\n", frame_type);
            
            // Print raw CinePak area
            print_hex_dump(cinepak_area, cinepak_size < 128 ? cinepak_size : 128, "Raw CinePak Data");
            
            // Search for patterns
            printf("  Searching for patterns:\n");
            
            // Look for 'cvid' signature
            bool found_cvid = false;
            for (uint32_t i = 0; i <= cinepak_size - 4; i++) {
                if (cinepak_area[i] == 0x63 && cinepak_area[i+1] == 0x76 && 
                    cinepak_area[i+2] == 0x69 && cinepak_area[i+3] == 0x64) {
                    printf("    Found 'cvid' at offset %u\n", i);
                    found_cvid = true;
                    print_hex_dump(cinepak_area + i, 32, "'cvid' area");
                }
            }
            
            if (!found_cvid) {
                printf("    No 'cvid' signature found\n");
            }
            
            // Analyze as potential CinePak header
            if (cinepak_size >= 10) {
                printf("  CinePak header analysis:\n");
                printf("    Bytes 0-1 (flags): 0x%02X%02X\n", cinepak_area[0], cinepak_area[1]);
                printf("    Bytes 2-5 (length): 0x%02X%02X%02X%02X (%u)\n", 
                       cinepak_area[2], cinepak_area[3], cinepak_area[4], cinepak_area[5],
                       (cinepak_area[2] << 24) | (cinepak_area[3] << 16) | (cinepak_area[4] << 8) | cinepak_area[5]);
                printf("    Bytes 6-9 (width/height): 0x%02X%02X%02X%02X\n", 
                       cinepak_area[6], cinepak_area[7], cinepak_area[8], cinepak_area[9]);
                
                uint16_t cv_width = (cinepak_area[6] << 8) | cinepak_area[7];
                uint16_t cv_height = (cinepak_area[8] << 8) | cinepak_area[9];
                printf("    Parsed dimensions: %ux%u\n", cv_width, cv_height);
            }
        }
    } else if (frame_sig == 0x46484452) { // "FHDR"
        printf("  -> FHDR frame (header only)\n");
        print_hex_dump(frame_data, data_size < 64 ? data_size : 64, "FHDR Data");
    }
    
    free(frame_data);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <3do_file>\n", argv[0]);
        return 1;
    }
    
    FILE* file = fopen(argv[1], "rb");
    if (!file) {
        printf("Cannot open file: %s\n", argv[1]);
        return 1;
    }
    
    printf("3DO Stream Analysis Tool\n");
    printf("========================\n");
    printf("Analyzing: %s\n\n", argv[1]);
    
    int chunk_count = 0;
    int film_count = 0;
    
    while (!feof(file)) {
        ChunkHeader header;
        
        if (fread(&header, sizeof(header), 1, file) != 1) break;
        
        // Convert from big-endian
        header.chunk_type = __builtin_bswap32(header.chunk_type);
        header.chunk_size = __builtin_bswap32(header.chunk_size);
        header.chunk_time = __builtin_bswap32(header.chunk_time);
        header.channel = __builtin_bswap32(header.channel);
        
        uint32_t data_size = header.chunk_size - 16; // Subtract header size
        
        chunk_count++;
        
        // Convert chunk type to string
        char chunk_name[5];
        chunk_name[0] = (header.chunk_type >> 24) & 0xFF;
        chunk_name[1] = (header.chunk_type >> 16) & 0xFF;
        chunk_name[2] = (header.chunk_type >> 8) & 0xFF;
        chunk_name[3] = header.chunk_type & 0xFF;
        chunk_name[4] = '\0';
        
        printf("Chunk %d: %s, size: %u, time: %u, data: %u bytes\n", 
               chunk_count, chunk_name, header.chunk_size, header.chunk_time, data_size);
        
        if (header.chunk_type == 0x46494C4D) { // "FILM"
            film_count++;
            printf("=== FILM CHUNK #%d ANALYSIS ===\n", film_count);
            analyze_film_chunk(file, data_size);
            printf("=== END FILM CHUNK #%d ===\n\n", film_count);
        } else {
            // Skip other chunk types
            fseek(file, data_size, SEEK_CUR);
        }
        
        // Limit analysis to prevent overwhelming output
        if (film_count >= 5) {
            printf("Reached analysis limit (5 FILM chunks)\n");
            break;
        }
    }
    
    fclose(file);
    printf("Analysis complete. Processed %d chunks, %d FILM chunks.\n", chunk_count, film_count);
    return 0;
}

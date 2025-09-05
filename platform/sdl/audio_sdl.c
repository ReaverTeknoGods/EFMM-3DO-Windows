#include "platform/platform_audio.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * SDL2_mixer audio implementation
 * Replaces 3DO audio system with modern cross-platform equivalent
 */

// Audio state
static bool g_audio_initialized = false;
static Mix_Chunk* g_ram_sounds[kMaxRamSounds];
static Mix_Music* g_current_music = NULL;
static int g_num_allocated_channels = 0;

// Sound spooling state
typedef struct SpoolSound {
    Mix_Music* music;
    char filename[256];
    bool is_loaded;
    bool is_playing;
    int loop_count;
} SpoolSound;

static SpoolSound g_spool_sounds[8];
static int g_num_spool_sounds = 0;

// Audio folio functions
int OpenAudioFolio(void)
{
    if (g_audio_initialized) {
        return 0; // Already initialized
    }

    // Initialize SDL audio subsystem
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("SDL_InitSubSystem(AUDIO) failed: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize SDL_mixer
    int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16SYS;
    int audio_channels = 2;
    int audio_buffers = 2048;

    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) < 0) {
        printf("Mix_OpenAudio failed: %s\n", Mix_GetError());
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return -1;
    }

    // Allocate mixing channels
    g_num_allocated_channels = Mix_AllocateChannels(kNumChannels);
    printf("Allocated %d audio channels\n", g_num_allocated_channels);

    // Initialize sound arrays
    memset(g_ram_sounds, 0, sizeof(g_ram_sounds));
    memset(g_spool_sounds, 0, sizeof(g_spool_sounds));

    g_audio_initialized = true;
    printf("Audio system initialized: %d Hz, %d channels, %d buffer size\n",
           audio_rate, audio_channels, audio_buffers);

    return 0;
}

void CloseAudioFolio(void)
{
    if (!g_audio_initialized) {
        return;
    }

    // Stop all sounds
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

    // Free RAM sounds
    for (int i = 0; i < kMaxRamSounds; i++) {
        if (g_ram_sounds[i]) {
            Mix_FreeChunk(g_ram_sounds[i]);
            g_ram_sounds[i] = NULL;
        }
    }

    // Free spool sounds
    for (int i = 0; i < g_num_spool_sounds; i++) {
        if (g_spool_sounds[i].music) {
            Mix_FreeMusic(g_spool_sounds[i].music);
            g_spool_sounds[i].music = NULL;
        }
    }

    if (g_current_music) {
        Mix_FreeMusic(g_current_music);
        g_current_music = NULL;
    }

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    g_audio_initialized = false;
    printf("Audio system shut down\n");
}

// Sound loading and management
int LoadRAMSound(const char* filename)
{
    if (!g_audio_initialized) {
        printf("Audio not initialized\n");
        return -1;
    }

    // Find empty slot
    int slot = -1;
    for (int i = 1; i < kMaxRamSounds; i++) { // Start at 1, reserve 0
        if (!g_ram_sounds[i]) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("No free sound slots\n");
        return -1;
    }

    // Build full path
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s%s", "$progdir/aiff/", filename);
    
    // Convert $progdir to actual path
    char* actual_path = full_path;
    if (strncmp(full_path, "$progdir", 8) == 0) {
        // Simple replacement - in real implementation, get actual program directory
        snprintf(full_path, sizeof(full_path), "./aiff/%s", filename);
        actual_path = full_path;
    }

    // Load sound file
    Mix_Chunk* chunk = Mix_LoadWAV(actual_path);
    if (!chunk) {
        // Try alternative formats
        char alt_path[512];
        snprintf(alt_path, sizeof(alt_path), "./sounds/%s", filename);
        chunk = Mix_LoadWAV(alt_path);
    }

    if (!chunk) {
        printf("Failed to load sound '%s': %s\n", actual_path, Mix_GetError());
        return -1;
    }

    g_ram_sounds[slot] = chunk;
    printf("Loaded sound %d: %s\n", slot, filename);
    return slot;
}

int UnloadRAMSound(int sound_id)
{
    if (sound_id < 0 || sound_id >= kMaxRamSounds || !g_ram_sounds[sound_id]) {
        return -1;
    }

    Mix_FreeChunk(g_ram_sounds[sound_id]);
    g_ram_sounds[sound_id] = NULL;
    return 0;
}

SoundFileRec* LoadSoundFile(const char* filename)
{
    // This is a simplified version - the original 3DO version was more complex
    SoundFileRec* rec = malloc(sizeof(SoundFileRec));
    if (!rec) return NULL;

    memset(rec, 0, sizeof(SoundFileRec));

    // In a full implementation, we'd load and parse the audio file here
    // For now, just store the filename
    printf("LoadSoundFile: %s (simplified)\n", filename);

    return rec;
}

void UnloadSoundFile(SoundFileRec* sound_file)
{
    if (sound_file) {
        if (sound_file->sound_data) {
            free(sound_file->sound_data);
        }
        free(sound_file);
    }
}

// Sound playback
int StartRAMSound(int sound_id, frac16 left_volume, frac16 right_volume, int32 rate)
{
    if (!g_audio_initialized || sound_id < 0 || sound_id >= kMaxRamSounds || !g_ram_sounds[sound_id]) {
        return -1;
    }

    // Convert frac16 volumes to 0-128 range
    int volume = ConvertF16_32(left_volume + right_volume) / 2;
    if (volume > 128) volume = 128;
    if (volume < 0) volume = 0;

    Mix_VolumeChunk(g_ram_sounds[sound_id], volume);

    // Play sound on first available channel
    int channel = Mix_PlayChannel(-1, g_ram_sounds[sound_id], 0);
    if (channel == -1) {
        printf("Failed to play sound %d: %s\n", sound_id, Mix_GetError());
        return -1;
    }

    return channel;
}

int StopRAMSound(int sound_id)
{
    // Stop all instances of this sound (simplified)
    // In a full implementation, we'd track which channels are playing which sounds
    Mix_HaltChannel(-1);
    return 0;
}

// Spooled sound (for music and large audio files)
int LoadSpoolSound(const char* filename)
{
    if (!g_audio_initialized) {
        return -1;
    }

    // Find empty slot
    int slot = -1;
    for (int i = 0; i < 8; i++) {
        if (!g_spool_sounds[i].is_loaded) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        printf("No free spool sound slots\n");
        return -1;
    }

    // Build full path
    char full_path[512];
    if (strncmp(filename, "$progdir", 8) == 0) {
        snprintf(full_path, sizeof(full_path), ".%s", filename + 8);
    } else {
        strncpy(full_path, filename, sizeof(full_path) - 1);
        full_path[sizeof(full_path) - 1] = '\0';
    }

    // Load music file
    Mix_Music* music = Mix_LoadMUS(full_path);
    if (!music) {
        printf("Failed to load music '%s': %s\n", full_path, Mix_GetError());
        return -1;
    }

    g_spool_sounds[slot].music = music;
    strncpy(g_spool_sounds[slot].filename, filename, sizeof(g_spool_sounds[slot].filename) - 1);
    g_spool_sounds[slot].is_loaded = true;
    g_spool_sounds[slot].is_playing = false;
    g_spool_sounds[slot].loop_count = 0;

    if (slot >= g_num_spool_sounds) {
        g_num_spool_sounds = slot + 1;
    }

    printf("Loaded spool sound %d: %s\n", slot, filename);
    return slot;
}

int UnloadSpoolSound(int spool_id)
{
    if (spool_id < 0 || spool_id >= g_num_spool_sounds || !g_spool_sounds[spool_id].is_loaded) {
        return -1;
    }

    if (g_spool_sounds[spool_id].is_playing) {
        Mix_HaltMusic();
    }

    Mix_FreeMusic(g_spool_sounds[spool_id].music);
    memset(&g_spool_sounds[spool_id], 0, sizeof(SpoolSound));
    return 0;
}

int StartSpoolSound(int spool_id, int32 loop_count)
{
    if (spool_id < 0 || spool_id >= g_num_spool_sounds || !g_spool_sounds[spool_id].is_loaded) {
        return -1;
    }

    // Stop current music
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }

    // Convert loop count (-1 = infinite, 0 = play once)
    int loops = (loop_count == 9999 || loop_count < 0) ? -1 : loop_count;

    if (Mix_PlayMusic(g_spool_sounds[spool_id].music, loops) == -1) {
        printf("Failed to play spool sound %d: %s\n", spool_id, Mix_GetError());
        return -1;
    }

    g_spool_sounds[spool_id].is_playing = true;
    g_spool_sounds[spool_id].loop_count = loop_count;

    printf("Started spool sound %d (%s) with %d loops\n", 
           spool_id, g_spool_sounds[spool_id].filename, loops);
    return 0;
}

int StopSpoolSound(int spool_id)
{
    if (spool_id < 0 || spool_id >= g_num_spool_sounds) {
        return -1;
    }

    if (g_spool_sounds[spool_id].is_playing) {
        Mix_HaltMusic();
        g_spool_sounds[spool_id].is_playing = false;
    }

    return 0;
}

int GetSpoolSoundStatus(int spool_id)
{
    if (spool_id < 0 || spool_id >= g_num_spool_sounds || !g_spool_sounds[spool_id].is_loaded) {
        return -1;
    }

    if (g_spool_sounds[spool_id].is_playing && Mix_PlayingMusic()) {
        return 1; // Playing
    } else {
        g_spool_sounds[spool_id].is_playing = false;
        return 0; // Stopped
    }
}

// Generic sound interface
int CallSound(CallSoundRec* call_rec)
{
    if (!call_rec) return -1;

    switch (call_rec->whatIWant) {
        case kInitializeSound:
            return OpenAudioFolio();

        case kCleanupSound:
            CloseAudioFolio();
            return 0;

        case kStartRAMSound: {
            RAMSoundRec* ram_rec = (RAMSoundRec*)call_rec;
            return StartRAMSound(ram_rec->soundID, ram_rec->leftVolume, ram_rec->rightVolume, ram_rec->rate);
        }

        case kStopRAMSound: {
            RAMSoundRec* ram_rec = (RAMSoundRec*)call_rec;
            return StopRAMSound(ram_rec->soundID);
        }

        default:
            printf("Unknown sound command: %d\n", call_rec->whatIWant);
            return -1;
    }
}

// Simplified instrument support (3DO had complex audio synthesis)
Item LoadInsTemplate(const char* template_name)
{
    printf("LoadInsTemplate: %s (simplified)\n", template_name);
    return 1; // Return dummy item
}

void UnloadInsTemplate(Item template_item)
{
    printf("UnloadInsTemplate: %d (simplified)\n", template_item);
}

Item CreateInstrument(Item template_item, void* tags)
{
    printf("CreateInstrument: template %d (simplified)\n", template_item);
    return template_item + 100; // Return dummy instrument item
}

void FreeInstrument(Item instrument_item)
{
    printf("FreeInstrument: %d (simplified)\n", instrument_item);
}

Item GrabKnob(Item instrument_item, const char* knob_name)
{
    printf("GrabKnob: instrument %d, knob %s (simplified)\n", instrument_item, knob_name);
    return instrument_item + 1000; // Return dummy knob item
}

void ReleaseKnob(Item knob_item)
{
    printf("ReleaseKnob: %d (simplified)\n", knob_item);
}

int TweakKnob(Item knob_item, frac16 value)
{
    // Convert frac16 to float for volume control
    float vol = (float)value / ONE_F16;
    int volume = (int)(vol * 128);
    if (volume > 128) volume = 128;
    if (volume < 0) volume = 0;

    // Apply to music volume (simplified)
    Mix_VolumeMusic(volume);
    return 0;
}

// Platform-specific audio functions
int platform_init_audio(void)
{
    return OpenAudioFolio();
}

void platform_shutdown_audio(void)
{
    CloseAudioFolio();
}

int platform_load_sound_file(const char* filename, void** data, int32* size)
{
    // Load raw audio data (simplified implementation)
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return -1;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *data = malloc(*size);
    if (!*data) {
        fclose(file);
        return -1;
    }

    fread(*data, 1, *size, file);
    fclose(file);

    return 0;
}

int platform_play_sound(int sound_id, float volume)
{
    if (sound_id < 0 || sound_id >= kMaxRamSounds || !g_ram_sounds[sound_id]) {
        return -1;
    }

    int vol = (int)(volume * 128);
    if (vol > 128) vol = 128;
    if (vol < 0) vol = 0;

    Mix_VolumeChunk(g_ram_sounds[sound_id], vol);
    return Mix_PlayChannel(-1, g_ram_sounds[sound_id], 0);
}

int platform_stop_sound(int sound_id)
{
    return StopRAMSound(sound_id);
}

// Audio format detection and conversion (simplified)
int platform_detect_audio_format(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    if (!ext) return -1;

#ifdef _WIN32
    if (_stricmp(ext, ".aifc") == 0) return AUDIO_FORMAT_AIFC;
    if (_stricmp(ext, ".aiff") == 0) return AUDIO_FORMAT_AIFF;
    if (_stricmp(ext, ".wav") == 0) return AUDIO_FORMAT_WAV;
    if (_stricmp(ext, ".ogg") == 0) return AUDIO_FORMAT_OGG;
#else
    if (strcasecmp(ext, ".aifc") == 0) return AUDIO_FORMAT_AIFC;
    if (strcasecmp(ext, ".aiff") == 0) return AUDIO_FORMAT_AIFF;
    if (strcasecmp(ext, ".wav") == 0) return AUDIO_FORMAT_WAV;
    if (strcasecmp(ext, ".ogg") == 0) return AUDIO_FORMAT_OGG;
#endif

    return -1;
}

int platform_convert_audio_format(void* input_data, int32 input_size, 
                                  AudioFormat input_format,
                                  void** output_data, int32* output_size,
                                  AudioFormat output_format)
{
    // Simplified - in a real implementation, we'd use a library like libsndfile
    printf("Audio format conversion not implemented (simplified)\n");
    *output_data = malloc(input_size);
    memcpy(*output_data, input_data, input_size);
    *output_size = input_size;
    return 0;
}

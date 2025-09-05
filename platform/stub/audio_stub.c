/*
 * audio_stub.c - Stub audio implementation for when SDL2_mixer is not available
 * This allows the project to compile and demonstrate the porting approach
 */

#include "../../threedo_compat.h"
#include <stdio.h>

static int32 stub_audio_initialized = 0;

int32 OpenAudioFolio(void) {
    printf("STUB: Opening audio folio\n");
    stub_audio_initialized = 1;
    return 0;
}

void CloseAudioFolio(void) {
    printf("STUB: Closing audio folio\n");
    stub_audio_initialized = 0;
}

int32 LoadRAMSound(const char* filename) {
    printf("STUB: Loading RAM sound: %s\n", filename ? filename : "NULL");
    return 1; // Return fake handle
}

int UnloadRAMSound(int soundHandle) {
    printf("STUB: Unloading RAM sound %d\n", soundHandle);
    return 0;
}

int32 PlayRAMSound(int32 soundHandle, int32 volume, int32 pan, int32 frequency) {
    printf("STUB: Playing RAM sound %d (vol: %d, pan: %d, freq: %d)\n", 
           soundHandle, volume, pan, frequency);
    return 1; // Return fake voice handle
}

int StopRAMSound(int32 voiceHandle) {
    printf("STUB: Stopping RAM sound voice %d\n", voiceHandle);
    return 0;
}

int32 StartSpoolSound(const char* filename, int32 bufferSize) {
    printf("STUB: Starting spooled sound: %s (buffer: %d)\n", 
           filename ? filename : "NULL", bufferSize);
    return 1; // Return fake handle
}

int StopSpoolSound(int32 spoolHandle) {
    printf("STUB: Stopping spooled sound %d\n", spoolHandle);
    return 0;
}

void SetSpoolSoundVolume(int32 spoolHandle, int32 volume) {
    printf("STUB: Setting spooled sound %d volume to %d\n", spoolHandle, volume);
}

int32 LoadMusic(const char* filename) {
    printf("STUB: Loading music: %s\n", filename ? filename : "NULL");
    return 1; // Return fake handle
}

void UnloadMusic(int32 musicHandle) {
    printf("STUB: Unloading music %d\n", musicHandle);
}

int32 PlayMusic(int32 musicHandle, int32 volume, int32 loop) {
    printf("STUB: Playing music %d (vol: %d, loop: %d)\n", musicHandle, volume, loop);
    return 0;
}

void StopMusic(int32 musicHandle) {
    printf("STUB: Stopping music %d\n", musicHandle);
}

void SetMusicVolume(int32 musicHandle, int32 volume) {
    printf("STUB: Setting music %d volume to %d\n", musicHandle, volume);
}

void PauseAudio(void) {
    printf("STUB: Pausing audio\n");
}

void ResumeAudio(void) {
    printf("STUB: Resuming audio\n");
}

void SetMasterVolume(int32 volume) {
    printf("STUB: Setting master volume to %d\n", volume);
}

int32 GetMasterVolume(void) {
    printf("STUB: Getting master volume (returning 100)\n");
    return 100;
}

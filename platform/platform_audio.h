#ifndef PLATFORM_AUDIO_H
#define PLATFORM_AUDIO_H

#include "platform_types.h"

/*
 * Cross-platform audio abstraction
 * Replaces 3DO audio.h functionality
 */

// Audio item structure (replaces 3DO audio Items)
typedef struct AudioItem {
    Item item_id;
    void* platform_data;
} AudioItem;

// Sound file record (replaces 3DO SoundFileRec) 
typedef struct SoundFileRec {
    void* sound_data;
    int32 sound_size;
    int32 sample_rate;
    int32 channels;
    int32 bits_per_sample;
} SoundFileRec;

// RAM sound record for quick playback
typedef struct RAMSoundRec {
    int32 whatIWant;     // Command type
    int32 soundID;       // Sound identifier
    frac16 leftVolume;   // Left channel volume
    frac16 rightVolume;  // Right channel volume
    int32 rate;          // Playback rate
} RAMSoundRec;

// Call sound record for general sound commands
typedef struct CallSoundRec {
    int32 whatIWant;     // Command type
    void* data;          // Command-specific data
} CallSoundRec;

// Sound command types
#define kInitializeSound    1
#define kCleanupSound      2
#define kLoadRAMSound      3
#define kUnloadRAMSound    4
#define kStartRAMSound     5
#define kStopRAMSound      6
#define kLoadSpoolSound    7
#define kUnloadSpoolSound  8
#define kStartSpoolSound   9
#define kStopSpoolSound    10
#define kSpoolSoundStatus  11

// Maximum number of sounds
#define kMaxRamSounds      32
#define kNumChannels       8

// Audio folio functions (replaces 3DO audio functions)
int OpenAudioFolio(void);
void CloseAudioFolio(void);

// Sound loading and management
int LoadRAMSound(const char* filename);
int UnloadRAMSound(int sound_id);
SoundFileRec* LoadSoundFile(const char* filename);
void UnloadSoundFile(SoundFileRec* sound_file);

// Sound playback
int StartRAMSound(int sound_id, frac16 left_volume, frac16 right_volume, int32 rate);
int StopRAMSound(int sound_id);

// Spooled sound (for music and large audio files)
int LoadSpoolSound(const char* filename);
int UnloadSpoolSound(int spool_id);
int StartSpoolSound(int spool_id, int32 loop_count);
int StopSpoolSound(int spool_id);
int GetSpoolSoundStatus(int spool_id);

// Generic sound interface
int CallSound(CallSoundRec* call_rec);

// Instrument and mixer support (3DO had complex audio synthesis)
typedef struct InstrumentItem {
    Item item_id;
    void* platform_data;
} InstrumentItem;

typedef struct KnobItem {
    Item item_id;
    void* platform_data;
} KnobItem;

// Audio synthesis functions (simplified for porting)
Item LoadInsTemplate(const char* template_name);
void UnloadInsTemplate(Item template_item);
Item CreateInstrument(Item template_item, void* tags);
void FreeInstrument(Item instrument_item);
Item GrabKnob(Item instrument_item, const char* knob_name);
void ReleaseKnob(Item knob_item);
int TweakKnob(Item knob_item, frac16 value);

// Audio streaming support
typedef struct AudioStreamContext {
    void* stream_data;
    int32 buffer_size;
    int32 sample_rate;
    bool is_active;
} AudioStreamContext;

int InitAudioStream(AudioStreamContext* ctx, int32 buffer_size, int32 sample_rate);
void CleanupAudioStream(AudioStreamContext* ctx);
int FeedAudioStream(AudioStreamContext* ctx, void* data, int32 size);

// Platform-specific audio initialization
int platform_init_audio(void);
void platform_shutdown_audio(void);
int platform_load_sound_file(const char* filename, void** data, int32* size);
int platform_play_sound(int sound_id, float volume);
int platform_stop_sound(int sound_id);

// Audio file format support
typedef enum AudioFormat {
    AUDIO_FORMAT_AIFC,
    AUDIO_FORMAT_AIFF, 
    AUDIO_FORMAT_WAV,
    AUDIO_FORMAT_OGG
} AudioFormat;

int platform_detect_audio_format(const char* filename);
int platform_convert_audio_format(void* input_data, int32 input_size, 
                                  AudioFormat input_format,
                                  void** output_data, int32* output_size,
                                  AudioFormat output_format);

#endif // PLATFORM_AUDIO_H

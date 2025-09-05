#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

#include "platform_types.h"

/*
 * Cross-platform input abstraction
 * Replaces 3DO event.h functionality
 */

// Control pad bit definitions (from 3DO)
#define ControlA            0x01
#define ControlB            0x02  
#define ControlC            0x04
#define ControlStart        0x08
#define ControlUp           0x10
#define ControlDown         0x20
#define ControlLeft         0x40
#define ControlRight        0x80
#define ControlLeftShift    0x100
#define ControlRightShift   0x200
#define ControlX            0x400

// Joypad data structure (replaces 3DO EventBroker)
typedef struct JoyData {
    uint32 jd_ButtonBits;     // Current button state
    int32  jd_DX, jd_DZ;      // Movement deltas  
    int32  jd_DAng;           // Rotation delta
    int32  jd_FrameCount;     // Frames since last update
    
    // Individual button states for easier access
    bool jd_ADown;
    bool jd_BDown;
    bool jd_CDown;
    bool jd_StartDown;
    bool jd_XDown;
    bool jd_UpDown;
    bool jd_DownDown;
    bool jd_LeftDown;
    bool jd_RightDown;
    bool jd_LeftShiftDown;
    bool jd_RightShiftDown;
} JoyData;

// Event structure (simplified from 3DO EventFrame)
typedef struct EventFrame {
    uint32 ef_EventBits;
    int32  ef_PenX, ef_PenY;
    uint32 ef_ButtonBits;
    int32  ef_AnalogX, ef_AnalogY;
} EventFrame;

// 3DO compatibility typedef with proper field names
typedef struct ControlPadEventData {
    uint32 cped_EventBits;
    int32  cped_PenX, cped_PenY;
    uint32 cped_ButtonBits;
    int32  cped_AnalogX, cped_AnalogY;
} ControlPadEventData;

// Input initialization and cleanup
int platform_init_input(void);
void platform_shutdown_input(void);

// Event handling functions
int GetControlPad(int32 pad_number, bool wait_for_edge, ControlPadEventData* event);
int platform_poll_events(void);
int platform_get_joypad_state(JoyData* joy_data);

// Keyboard mapping for PC controls
typedef enum KeyCode {
    KEY_UNKNOWN = 0,
    KEY_W, KEY_A, KEY_S, KEY_D,     // Movement
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, // Arrow keys
    KEY_SPACE, KEY_ENTER, KEY_ESC,   // Action keys
    KEY_Z, KEY_X, KEY_C,             // ABC buttons
    KEY_TAB, KEY_SHIFT,              // Shift buttons
    KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10
} KeyCode;

// Mouse input support
typedef struct MouseState {
    int32 x, y;              // Current position
    int32 delta_x, delta_y;  // Movement since last update
    bool left_button;
    bool right_button;
    bool middle_button;
    int32 wheel_delta;
} MouseState;

// Gamepad support (for modern controllers)
typedef struct GamepadState {
    bool connected;
    float left_stick_x, left_stick_y;
    float right_stick_x, right_stick_y;
    float left_trigger, right_trigger;
    
    bool button_a, button_b, button_x, button_y;
    bool button_start, button_select;
    bool button_left_shoulder, button_right_shoulder;
    bool dpad_up, dpad_down, dpad_left, dpad_right;
    bool left_stick_button, right_stick_button;
} GamepadState;

// Input state management
int platform_update_input_state(void);
bool platform_is_key_down(KeyCode key);
bool platform_is_key_pressed(KeyCode key);   // True only on first frame of press
bool platform_is_key_released(KeyCode key);  // True only on first frame of release
int platform_get_mouse_state(MouseState* mouse);
int platform_get_gamepad_state(int gamepad_index, GamepadState* gamepad);

// Input mapping functions
uint32 platform_map_keyboard_to_joypad(void);
uint32 platform_map_gamepad_to_joypad(int gamepad_index);
void platform_update_joydata_from_input(JoyData* joy_data);

// Thread support for input polling (replaces 3DO thread system)
typedef void (*ThreadFunc)(void);

Item CreateThread(const char* name, int32 priority, ThreadFunc func, int32 stack_size);
void DeleteThread(Item thread_item);

// Input configuration
typedef struct InputConfig {
    KeyCode move_forward;
    KeyCode move_backward;
    KeyCode move_left;
    KeyCode move_right;
    KeyCode turn_left;
    KeyCode turn_right;
    KeyCode action_a;
    KeyCode action_b;
    KeyCode action_c;
    KeyCode pause;
    KeyCode map;
    bool mouse_look;
    float mouse_sensitivity;
    bool gamepad_enabled;
    float gamepad_sensitivity;
} InputConfig;

// Default input configuration
extern InputConfig g_default_input_config;

// Input configuration functions
void platform_load_input_config(const char* filename);
void platform_save_input_config(const char* filename);
void platform_reset_input_config_to_defaults(void);

// Text input support (for save game names, etc.)
typedef struct TextInput {
    char buffer[256];
    int32 cursor_position;
    int32 max_length;
    bool active;
} TextInput;

void platform_start_text_input(TextInput* input, int32 max_length);
void platform_stop_text_input(TextInput* input);
void platform_update_text_input(TextInput* input);

#endif // PLATFORM_INPUT_H

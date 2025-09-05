#include "platform/platform_input.h"
#include <SDL.h>
#include <stdio.h>
#include <string.h>

/*
 * SDL2 input implementation
 * Replaces 3DO input system with modern cross-platform equivalent
 */

// Input state
static bool g_input_initialized = false;
static JoyData g_current_joydata = {0};
static JoyData g_previous_joydata = {0};
static InputConfig g_input_config;
static bool g_keys_current[SDL_NUM_SCANCODES] = {0};
static bool g_keys_previous[SDL_NUM_SCANCODES] = {0};
static MouseState g_mouse_state = {0};
static GamepadState g_gamepad_states[4] = {0};
static SDL_GameController* g_controllers[4] = {NULL};
static Uint32 g_last_frame_time = 0;

// Default input configuration
InputConfig g_default_input_config = {
    .move_forward = KEY_W,
    .move_backward = KEY_S,
    .move_left = KEY_A,
    .move_right = KEY_D,
    .turn_left = KEY_LEFT,
    .turn_right = KEY_RIGHT,
    .action_a = KEY_SPACE,
    .action_b = KEY_Z,
    .action_c = KEY_X,
    .pause = KEY_ENTER,
    .map = KEY_TAB,
    .mouse_look = true,
    .mouse_sensitivity = 1.0f,
    .gamepad_enabled = true,
    .gamepad_sensitivity = 1.0f
};

// Utility function to convert SDL scancode to our KeyCode
static KeyCode sdl_scancode_to_keycode(SDL_Scancode scancode)
{
    switch (scancode) {
        case SDL_SCANCODE_W: return KEY_W;
        case SDL_SCANCODE_A: return KEY_A;
        case SDL_SCANCODE_S: return KEY_S;
        case SDL_SCANCODE_D: return KEY_D;
        case SDL_SCANCODE_UP: return KEY_UP;
        case SDL_SCANCODE_DOWN: return KEY_DOWN;
        case SDL_SCANCODE_LEFT: return KEY_LEFT;
        case SDL_SCANCODE_RIGHT: return KEY_RIGHT;
        case SDL_SCANCODE_SPACE: return KEY_SPACE;
        case SDL_SCANCODE_RETURN: return KEY_ENTER;
        case SDL_SCANCODE_ESCAPE: return KEY_ESC;
        case SDL_SCANCODE_Z: return KEY_Z;
        case SDL_SCANCODE_X: return KEY_X;
        case SDL_SCANCODE_C: return KEY_C;
        case SDL_SCANCODE_TAB: return KEY_TAB;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT: return KEY_SHIFT;
        default: return KEY_UNKNOWN;
    }
}

// Input initialization
int platform_init_input(void)
{
    if (g_input_initialized) {
        return 0;
    }

    // Initialize SDL game controller subsystem
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL_InitSubSystem(GAMECONTROLLER) failed: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize game controllers
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            g_controllers[i] = SDL_GameControllerOpen(i);
            if (g_controllers[i]) {
                printf("Game Controller %d connected: %s\n", i, SDL_GameControllerName(g_controllers[i]));
                g_gamepad_states[i].connected = true;
            }
        }
    }

    // Load default input configuration
    g_input_config = g_default_input_config;

    // Initialize timing
    g_last_frame_time = SDL_GetTicks();

    g_input_initialized = true;
    printf("Input system initialized\n");
    return 0;
}

void platform_shutdown_input(void)
{
    if (!g_input_initialized) {
        return;
    }

    // Close game controllers
    for (int i = 0; i < 4; i++) {
        if (g_controllers[i]) {
            SDL_GameControllerClose(g_controllers[i]);
            g_controllers[i] = NULL;
        }
    }

    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    g_input_initialized = false;
    printf("Input system shut down\n");
}

// Event handling
int platform_poll_events(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                return 1; // Signal to quit

            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (event.key.keysym.scancode < SDL_NUM_SCANCODES) {
                    g_keys_current[event.key.keysym.scancode] = (event.type == SDL_KEYDOWN);
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                bool pressed = (event.type == SDL_MOUSEBUTTONDOWN);
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        g_mouse_state.left_button = pressed;
                        break;
                    case SDL_BUTTON_RIGHT:
                        g_mouse_state.right_button = pressed;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        g_mouse_state.middle_button = pressed;
                        break;
                }
                break;
            }

            case SDL_MOUSEMOTION:
                g_mouse_state.delta_x = event.motion.xrel;
                g_mouse_state.delta_y = event.motion.yrel;
                g_mouse_state.x = event.motion.x;
                g_mouse_state.y = event.motion.y;
                break;

            case SDL_MOUSEWHEEL:
                g_mouse_state.wheel_delta = event.wheel.y;
                break;

            case SDL_CONTROLLERDEVICEADDED: {
                int device_index = event.cdevice.which;
                if (device_index < 4 && !g_controllers[device_index]) {
                    g_controllers[device_index] = SDL_GameControllerOpen(device_index);
                    if (g_controllers[device_index]) {
                        printf("Controller %d connected\n", device_index);
                        g_gamepad_states[device_index].connected = true;
                    }
                }
                break;
            }

            case SDL_CONTROLLERDEVICEREMOVED: {
                SDL_JoystickID instance_id = event.cdevice.which;
                for (int i = 0; i < 4; i++) {
                    if (g_controllers[i] && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(g_controllers[i])) == instance_id) {
                        SDL_GameControllerClose(g_controllers[i]);
                        g_controllers[i] = NULL;
                        g_gamepad_states[i].connected = false;
                        printf("Controller %d disconnected\n", i);
                        break;
                    }
                }
                break;
            }
        }
    }

    return 0; // Continue running
}

// Input state management
int platform_update_input_state(void)
{
    if (!g_input_initialized) {
        return -1;
    }

    // Copy current to previous
    memcpy(g_keys_previous, g_keys_current, sizeof(g_keys_current));
    g_previous_joydata = g_current_joydata;

    // Reset mouse deltas
    g_mouse_state.delta_x = 0;
    g_mouse_state.delta_y = 0;
    g_mouse_state.wheel_delta = 0;

    // Update gamepad states
    for (int i = 0; i < 4; i++) {
        if (g_controllers[i]) {
            GamepadState* pad = &g_gamepad_states[i];
            
            pad->left_stick_x = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
            pad->left_stick_y = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
            pad->right_stick_x = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
            pad->right_stick_y = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
            pad->left_trigger = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
            pad->right_trigger = SDL_GameControllerGetAxis(g_controllers[i], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
            
            pad->button_a = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_A);
            pad->button_b = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_B);
            pad->button_x = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_X);
            pad->button_y = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_Y);
            pad->button_start = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_START);
            pad->button_select = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_BACK);
            pad->button_left_shoulder = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            pad->button_right_shoulder = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            pad->dpad_up = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_UP);
            pad->dpad_down = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            pad->dpad_left = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            pad->dpad_right = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            pad->left_stick_button = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_LEFTSTICK);
            pad->right_stick_button = SDL_GameControllerGetButton(g_controllers[i], SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        }
    }

    return 0;
}

bool platform_is_key_down(KeyCode key)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    
    switch (key) {
        case KEY_W: scancode = SDL_SCANCODE_W; break;
        case KEY_A: scancode = SDL_SCANCODE_A; break;
        case KEY_S: scancode = SDL_SCANCODE_S; break;
        case KEY_D: scancode = SDL_SCANCODE_D; break;
        case KEY_UP: scancode = SDL_SCANCODE_UP; break;
        case KEY_DOWN: scancode = SDL_SCANCODE_DOWN; break;
        case KEY_LEFT: scancode = SDL_SCANCODE_LEFT; break;
        case KEY_RIGHT: scancode = SDL_SCANCODE_RIGHT; break;
        case KEY_SPACE: scancode = SDL_SCANCODE_SPACE; break;
        case KEY_ENTER: scancode = SDL_SCANCODE_RETURN; break;
        case KEY_ESC: scancode = SDL_SCANCODE_ESCAPE; break;
        case KEY_Z: scancode = SDL_SCANCODE_Z; break;
        case KEY_X: scancode = SDL_SCANCODE_X; break;
        case KEY_C: scancode = SDL_SCANCODE_C; break;
        case KEY_TAB: scancode = SDL_SCANCODE_TAB; break;
        case KEY_SHIFT: scancode = SDL_SCANCODE_LSHIFT; break;
        default: return false;
    }

    return g_keys_current[scancode];
}

bool platform_is_key_pressed(KeyCode key)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    
    switch (key) {
        case KEY_W: scancode = SDL_SCANCODE_W; break;
        case KEY_A: scancode = SDL_SCANCODE_A; break;
        case KEY_S: scancode = SDL_SCANCODE_S; break;
        case KEY_D: scancode = SDL_SCANCODE_D; break;
        case KEY_UP: scancode = SDL_SCANCODE_UP; break;
        case KEY_DOWN: scancode = SDL_SCANCODE_DOWN; break;
        case KEY_LEFT: scancode = SDL_SCANCODE_LEFT; break;
        case KEY_RIGHT: scancode = SDL_SCANCODE_RIGHT; break;
        case KEY_SPACE: scancode = SDL_SCANCODE_SPACE; break;
        case KEY_ENTER: scancode = SDL_SCANCODE_RETURN; break;
        case KEY_ESC: scancode = SDL_SCANCODE_ESCAPE; break;
        case KEY_Z: scancode = SDL_SCANCODE_Z; break;
        case KEY_X: scancode = SDL_SCANCODE_X; break;
        case KEY_C: scancode = SDL_SCANCODE_C; break;
        case KEY_TAB: scancode = SDL_SCANCODE_TAB; break;
        case KEY_SHIFT: scancode = SDL_SCANCODE_LSHIFT; break;
        default: return false;
    }

    return g_keys_current[scancode] && !g_keys_previous[scancode];
}

bool platform_is_key_released(KeyCode key)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    
    switch (key) {
        case KEY_W: scancode = SDL_SCANCODE_W; break;
        case KEY_A: scancode = SDL_SCANCODE_A; break;
        case KEY_S: scancode = SDL_SCANCODE_S; break;
        case KEY_D: scancode = SDL_SCANCODE_D; break;
        case KEY_UP: scancode = SDL_SCANCODE_UP; break;
        case KEY_DOWN: scancode = SDL_SCANCODE_DOWN; break;
        case KEY_LEFT: scancode = SDL_SCANCODE_LEFT; break;
        case KEY_RIGHT: scancode = SDL_SCANCODE_RIGHT; break;
        case KEY_SPACE: scancode = SDL_SCANCODE_SPACE; break;
        case KEY_ENTER: scancode = SDL_SCANCODE_RETURN; break;
        case KEY_ESC: scancode = SDL_SCANCODE_ESCAPE; break;
        case KEY_Z: scancode = SDL_SCANCODE_Z; break;
        case KEY_X: scancode = SDL_SCANCODE_X; break;
        case KEY_C: scancode = SDL_SCANCODE_C; break;
        case KEY_TAB: scancode = SDL_SCANCODE_TAB; break;
        case KEY_SHIFT: scancode = SDL_SCANCODE_LSHIFT; break;
        default: return false;
    }

    return !g_keys_current[scancode] && g_keys_previous[scancode];
}

int platform_get_mouse_state(MouseState* mouse)
{
    if (mouse) {
        *mouse = g_mouse_state;
    }
    return 0;
}

int platform_get_gamepad_state(int gamepad_index, GamepadState* gamepad)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || !gamepad) {
        return -1;
    }

    *gamepad = g_gamepad_states[gamepad_index];
    return 0;
}

// Convert modern input to 3DO joypad format
uint32 platform_map_keyboard_to_joypad(void)
{
    uint32 button_bits = 0;

    // Movement
    if (platform_is_key_down(g_input_config.move_left) || platform_is_key_down(g_input_config.turn_left)) {
        button_bits |= ControlLeft;
    }
    if (platform_is_key_down(g_input_config.move_right) || platform_is_key_down(g_input_config.turn_right)) {
        button_bits |= ControlRight;
    }
    if (platform_is_key_down(g_input_config.move_forward)) {
        button_bits |= ControlUp;
    }
    if (platform_is_key_down(g_input_config.move_backward)) {
        button_bits |= ControlDown;
    }

    // Action buttons
    if (platform_is_key_down(g_input_config.action_a)) {
        button_bits |= ControlA;
    }
    if (platform_is_key_down(g_input_config.action_b)) {
        button_bits |= ControlB;
    }
    if (platform_is_key_down(g_input_config.action_c)) {
        button_bits |= ControlC;
    }
    if (platform_is_key_down(g_input_config.pause)) {
        button_bits |= ControlStart;
    }
    if (platform_is_key_down(g_input_config.map)) {
        button_bits |= ControlX;
    }

    // Shift buttons
    if (platform_is_key_down(KEY_SHIFT)) {
        button_bits |= ControlLeftShift;
    }

    return button_bits;
}

uint32 platform_map_gamepad_to_joypad(int gamepad_index)
{
    if (gamepad_index < 0 || gamepad_index >= 4 || !g_gamepad_states[gamepad_index].connected) {
        return 0;
    }

    GamepadState* pad = &g_gamepad_states[gamepad_index];
    uint32 button_bits = 0;

    // D-pad
    if (pad->dpad_left || pad->left_stick_x < -0.5f) {
        button_bits |= ControlLeft;
    }
    if (pad->dpad_right || pad->left_stick_x > 0.5f) {
        button_bits |= ControlRight;
    }
    if (pad->dpad_up || pad->left_stick_y < -0.5f) {
        button_bits |= ControlUp;
    }
    if (pad->dpad_down || pad->left_stick_y > 0.5f) {
        button_bits |= ControlDown;
    }

    // Action buttons
    if (pad->button_a) {
        button_bits |= ControlA;
    }
    if (pad->button_b) {
        button_bits |= ControlB;
    }
    if (pad->button_x) {
        button_bits |= ControlC;
    }
    if (pad->button_start) {
        button_bits |= ControlStart;
    }
    if (pad->button_select) {
        button_bits |= ControlX;
    }

    // Shoulder buttons
    if (pad->button_left_shoulder) {
        button_bits |= ControlLeftShift;
    }
    if (pad->button_right_shoulder) {
        button_bits |= ControlRightShift;
    }

    return button_bits;
}

void platform_update_joydata_from_input(JoyData* joy_data)
{
    if (!joy_data) return;

    // Calculate frame timing
    Uint32 current_time = SDL_GetTicks();
    Uint32 delta_time = current_time - g_last_frame_time;
    joy_data->jd_FrameCount = (delta_time * 60) / 1000; // Convert to 60Hz frame count
    if (joy_data->jd_FrameCount < 1) joy_data->jd_FrameCount = 1;
    g_last_frame_time = current_time;

    // Reset deltas
    joy_data->jd_DX = joy_data->jd_DZ = joy_data->jd_DAng = 0;

    // Combine keyboard and gamepad input
    uint32 button_bits = platform_map_keyboard_to_joypad();
    if (g_input_config.gamepad_enabled) {
        button_bits |= platform_map_gamepad_to_joypad(0); // Use first gamepad
    }

    joy_data->jd_ButtonBits = button_bits;

    // Update individual button states
    joy_data->jd_ADown = (button_bits & ControlA) != 0;
    joy_data->jd_BDown = (button_bits & ControlB) != 0;
    joy_data->jd_CDown = (button_bits & ControlC) != 0;
    joy_data->jd_StartDown = (button_bits & ControlStart) != 0;
    joy_data->jd_XDown = (button_bits & ControlX) != 0;
    joy_data->jd_UpDown = (button_bits & ControlUp) != 0;
    joy_data->jd_DownDown = (button_bits & ControlDown) != 0;
    joy_data->jd_LeftDown = (button_bits & ControlLeft) != 0;
    joy_data->jd_RightDown = (button_bits & ControlRight) != 0;
    joy_data->jd_LeftShiftDown = (button_bits & ControlLeftShift) != 0;
    joy_data->jd_RightShiftDown = (button_bits & ControlRightShift) != 0;

    // Calculate movement deltas
    const frac16 MOVE_SPEED = ONE_F16 >> 4;
    const frac16 TURN_SPEED = ONE_F16 >> 6;

    if (joy_data->jd_UpDown) {
        joy_data->jd_DZ += MOVE_SPEED;
    }
    if (joy_data->jd_DownDown) {
        joy_data->jd_DZ -= MOVE_SPEED;
    }
    if (joy_data->jd_LeftDown) {
        if (joy_data->jd_LeftShiftDown) {
            joy_data->jd_DX -= MOVE_SPEED; // Strafe
        } else {
            joy_data->jd_DAng -= TURN_SPEED; // Turn
        }
    }
    if (joy_data->jd_RightDown) {
        if (joy_data->jd_RightShiftDown) {
            joy_data->jd_DX += MOVE_SPEED; // Strafe
        } else {
            joy_data->jd_DAng += TURN_SPEED; // Turn
        }
    }

    // Mouse look support
    if (g_input_config.mouse_look && (g_mouse_state.delta_x != 0 || g_mouse_state.delta_y != 0)) {
        joy_data->jd_DAng += (g_mouse_state.delta_x * g_input_config.mouse_sensitivity * TURN_SPEED) / 10;
        // Y movement could control pitch if the game supported it
    }

    // Gamepad analog support
    if (g_input_config.gamepad_enabled && g_gamepad_states[0].connected) {
        GamepadState* pad = &g_gamepad_states[0];
        
        // Add analog movement
        joy_data->jd_DX += (frac16)(pad->left_stick_x * MOVE_SPEED * g_input_config.gamepad_sensitivity);
        joy_data->jd_DZ += (frac16)(-pad->left_stick_y * MOVE_SPEED * g_input_config.gamepad_sensitivity);
        joy_data->jd_DAng += (frac16)(pad->right_stick_x * TURN_SPEED * g_input_config.gamepad_sensitivity);
    }

    g_current_joydata = *joy_data;
}

// 3DO compatibility functions
int GetControlPad(int32 pad_number, bool wait_for_edge, ControlPadEventData* event)
{
    (void)pad_number; // Unused
    (void)wait_for_edge; // Unused
    
    // Update joypad data from current input state
    JoyData temp_joydata;
    platform_update_joydata_from_input(&temp_joydata);
    
    if (event) {
        memset(event, 0, sizeof(ControlPadEventData));
        event->cped_ButtonBits = g_current_joydata.jd_ButtonBits;
        event->cped_PenX = g_mouse_state.x;
        event->cped_PenY = g_mouse_state.y;
    }
    
    return 0;
}

int platform_get_joypad_state(JoyData* joy_data)
{
    if (joy_data) {
        *joy_data = g_current_joydata;
    }
    return 0;
}

// Thread support (simplified)
Item CreateThread(const char* name, int32 priority, ThreadFunc func, int32 stack_size)
{
    (void)priority; // Unused
    (void)stack_size; // Unused
    
    printf("CreateThread: %s (simplified - running on main thread)\n", name);
    // In a full implementation, we'd create an actual thread here
    // For now, just call the function directly in some contexts
    return 1; // Return dummy thread item
}

void DeleteThread(Item thread_item)
{
    printf("DeleteThread: %d (simplified)\n", thread_item);
}

// Input configuration functions
void platform_load_input_config(const char* filename)
{
    printf("Loading input config from %s (not implemented)\n", filename);
    // In a full implementation, load from file
    g_input_config = g_default_input_config;
}

void platform_save_input_config(const char* filename)
{
    printf("Saving input config to %s (not implemented)\n", filename);
    // In a full implementation, save to file
}

void platform_reset_input_config_to_defaults(void)
{
    g_input_config = g_default_input_config;
    printf("Input configuration reset to defaults\n");
}

// Text input support (simplified)
void platform_start_text_input(TextInput* input, int32 max_length)
{
    if (input) {
        memset(input->buffer, 0, sizeof(input->buffer));
        input->cursor_position = 0;
        input->max_length = max_length < 255 ? max_length : 255;
        input->active = true;
        SDL_StartTextInput();
    }
}

void platform_stop_text_input(TextInput* input)
{
    if (input) {
        input->active = false;
        SDL_StopTextInput();
    }
}

void platform_update_text_input(TextInput* input)
{
    if (!input || !input->active) return;
    
    // In a full implementation, we'd handle SDL_TEXTINPUT events
    // For now, this is just a placeholder
}

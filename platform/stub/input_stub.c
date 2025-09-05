/*
 * input_stub.c - Stub input implementation for when SDL2 is not available
 * This allows the project to compile and demonstrate the porting approach
 */

#include "../../threedo_compat.h"
#include <stdio.h>
#include <string.h>

static JoyData stub_joydata = {0};
static int32 stub_key_states[256] = {0};

int32 platform_init_input(void) {
    printf("STUB: Initializing input\n");
    memset(&stub_joydata, 0, sizeof(JoyData));
    memset(stub_key_states, 0, sizeof(stub_key_states));
    return 0;
}

void platform_cleanup_input(void) {
    printf("STUB: Cleaning up input\n");
}

void platform_update_input(void) {
    // Simulate some basic input for testing
    static int32 frame_count = 0;
    frame_count++;
    
    // Simulate walking forward occasionally
    if ((frame_count % 120) < 60) {
        stub_joydata.jd_DZ = ONE_F16 >> 3; // Move forward slowly
    } else {
        stub_joydata.jd_DZ = 0;
    }
    
    // Simulate turning occasionally
    if ((frame_count % 200) < 30) {
        stub_joydata.jd_DAng = ONE_F16 >> 7; // Turn slowly
    } else {
        stub_joydata.jd_DAng = 0;
    }
    
    stub_joydata.jd_FrameCount = 1;
    
    if (frame_count % 60 == 0) {
        printf("STUB: Input update - frame %d, DZ=%d, DAng=%d\n", 
               frame_count, stub_joydata.jd_DZ, stub_joydata.jd_DAng);
    }
}

void platform_update_joydata_from_input(JoyData* joydata) {
    if (joydata) {
        *joydata = stub_joydata;
    }
}

bool platform_is_key_pressed(int32 keycode) {
    // Simulate some keys being pressed occasionally
    static int32 sim_frame = 0;
    sim_frame++;
    
    switch (keycode) {
        case 'W': case 'w':
            return (sim_frame % 180) < 60; // W pressed 1/3 of the time
        case 'S': case 's':
            return (sim_frame % 200) < 40; // S pressed 1/5 of the time
        case 'A': case 'a':
            return (sim_frame % 250) < 30; // A pressed occasionally
        case 'D': case 'd':
            return (sim_frame % 220) < 30; // D pressed occasionally
        case ' ': // Space
            return (sim_frame % 300) < 10; // Space pressed briefly
        default:
            return false;
    }
}

bool platform_is_key_down(int32 keycode) {
    return platform_is_key_pressed(keycode);
}

bool platform_is_key_up(int32 keycode) {
    return !platform_is_key_pressed(keycode);
}

int platform_get_mouse_state(int32* x, int32* y, uint32* buttons) {
    static int32 mouse_x = 160, mouse_y = 120;
    static uint32 mouse_buttons = 0;
    
    if (x) *x = mouse_x;
    if (y) *y = mouse_y;
    if (buttons) *buttons = mouse_buttons;
    return 0;
}

int32 platform_should_quit(void) {
    // Never quit in stub mode, let it run for demonstration
    return 0;
}

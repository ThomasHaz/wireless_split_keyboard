/**
 * Keyboard Dongle
 * Receives key events from both halves via BLE UART
 * Processes layers, macros, and sends USB HID to computer
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include "tusb.h"
#include "hardware/gpio.h"

// Key event structure (must match keyboard halves)
typedef struct {
    uint8_t type;      // 0 = key press, 1 = key release
    uint8_t row;
    uint8_t col;
    uint8_t side;      // 0 = left, 1 = right
} key_event_t;

// Keyboard dimensions
#define ROWS 5
#define COLS 7
#define SIDES 2
#define MAX_LAYERS 4

// Current layer
static uint8_t current_layer = 0;

// Key states for each side
static bool key_state[SIDES][ROWS][COLS] = {0};

// USB HID keyboard report
static uint8_t kbd_report[8] = {0};  // Modifier, reserved, 6 keys
static bool report_changed = false;

// Mouse state
static int8_t mouse_x = 0, mouse_y = 0;
static uint8_t mouse_buttons = 0;
static bool mouse_report_pending = false;

// Auto-click state
static bool auto_click_active = false;
static uint32_t auto_click_interval = 100;  // ms
static uint32_t last_auto_click = 0;

// Macro support
#define MAX_MACRO_LENGTH 32
#define MAX_MACROS 16

typedef struct {
    uint8_t length;
    uint8_t keys[MAX_MACRO_LENGTH];
} macro_t;

static macro_t macros[MAX_MACROS] = {0};
static int active_macro = -1;
static uint8_t macro_position = 0;
static uint32_t last_macro_time = 0;
#define MACRO_KEY_DELAY 50  // ms between macro keys

// HID keycodes (subset - expand as needed)
#define HID_KEY_A 0x04
#define HID_KEY_B 0x05
#define HID_KEY_C 0x06
#define HID_KEY_D 0x07
#define HID_KEY_E 0x08
#define HID_KEY_F 0x09
#define HID_KEY_G 0x0A
#define HID_KEY_H 0x0B
#define HID_KEY_I 0x0C
#define HID_KEY_J 0x0D
#define HID_KEY_K 0x0E
#define HID_KEY_L 0x0F
#define HID_KEY_M 0x10
#define HID_KEY_N 0x11
#define HID_KEY_O 0x12
#define HID_KEY_P 0x13
#define HID_KEY_Q 0x14
#define HID_KEY_R 0x15
#define HID_KEY_S 0x16
#define HID_KEY_T 0x17
#define HID_KEY_U 0x18
#define HID_KEY_V 0x19
#define HID_KEY_W 0x1A
#define HID_KEY_X 0x1B
#define HID_KEY_Y 0x1C
#define HID_KEY_Z 0x1D
#define HID_KEY_1 0x1E
#define HID_KEY_2 0x1F
#define HID_KEY_3 0x20
#define HID_KEY_4 0x21
#define HID_KEY_5 0x22
#define HID_KEY_6 0x23
#define HID_KEY_7 0x24
#define HID_KEY_8 0x25
#define HID_KEY_9 0x26
#define HID_KEY_0 0x27
#define HID_KEY_ENTER 0x28
#define HID_KEY_ESC 0x29
#define HID_KEY_BACKSPACE 0x2A
#define HID_KEY_TAB 0x2B
#define HID_KEY_SPACE 0x2C
#define HID_KEY_CAPS_LOCK 0x39
#define HID_KEY_F1 0x3A
#define HID_KEY_F2 0x3B
#define HID_KEY_F3 0x3C
#define HID_KEY_F4 0x3D
#define HID_KEY_F5 0x3E
#define HID_KEY_F6 0x3F
#define HID_KEY_F7 0x40
#define HID_KEY_F8 0x41
#define HID_KEY_F9 0x42
#define HID_KEY_F10 0x43
#define HID_KEY_F11 0x44
#define HID_KEY_F12 0x45
#define HID_KEY_DELETE 0x4C
#define HID_KEY_ARROW_RIGHT 0x4F
#define HID_KEY_ARROW_LEFT 0x50
#define HID_KEY_ARROW_DOWN 0x51
#define HID_KEY_ARROW_UP 0x52

// Modifiers
#define HID_MOD_LEFT_CTRL 0x01
#define HID_MOD_LEFT_SHIFT 0x02
#define HID_MOD_LEFT_ALT 0x04
#define HID_MOD_LEFT_GUI 0x08
#define HID_MOD_RIGHT_CTRL 0x10
#define HID_MOD_RIGHT_SHIFT 0x20
#define HID_MOD_RIGHT_ALT 0x40
#define HID_MOD_RIGHT_GUI 0x80

// Special key codes for layers and macros
#define KEY_LAYER_1 0xF0
#define KEY_LAYER_2 0xF1
#define KEY_LAYER_3 0xF2
#define KEY_MACRO_0 0xE0
#define KEY_MOUSE_LEFT 0xD0
#define KEY_MOUSE_RIGHT 0xD1
#define KEY_MOUSE_MIDDLE 0xD2
#define KEY_MOUSE_UP 0xD3
#define KEY_MOUSE_DOWN 0xD4
#define KEY_MOUSE_LEFT_MOVE 0xD5
#define KEY_MOUSE_RIGHT_MOVE 0xD6
#define KEY_AUTO_CLICK 0xD7

// Keymap - [layer][side][row][col]
// This is an example QWERTY layout - customize to your needs
static const uint8_t keymap[MAX_LAYERS][SIDES][ROWS][COLS] = {
    // Layer 0 - Base
    {
        // Left half
        {
            {HID_KEY_ESC, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, 0},
            {HID_KEY_TAB, HID_KEY_Q, HID_KEY_W, HID_KEY_E, HID_KEY_R, HID_KEY_T, 0},
            {HID_KEY_CAPS_LOCK, HID_KEY_A, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, 0},
            {HID_MOD_LEFT_SHIFT, HID_KEY_Z, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_B, 0},
            {HID_MOD_LEFT_CTRL, HID_MOD_LEFT_GUI, HID_MOD_LEFT_ALT, KEY_LAYER_1, HID_KEY_SPACE, 0, 0}
        },
        // Right half
        {
            {0, HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_0, HID_KEY_BACKSPACE},
            {0, HID_KEY_Y, HID_KEY_U, HID_KEY_I, HID_KEY_O, HID_KEY_P, HID_KEY_BACKSPACE},
            {0, HID_KEY_H, HID_KEY_J, HID_KEY_K, HID_KEY_L, 0, HID_KEY_ENTER},
            {0, HID_KEY_N, HID_KEY_M, 0, 0, HID_MOD_RIGHT_SHIFT, 0},
            {0, 0, HID_KEY_SPACE, KEY_LAYER_2, HID_MOD_RIGHT_ALT, HID_MOD_RIGHT_CTRL, 0}
        }
    },
    // Layer 1 - Function/Navigation
    {
        // Left half
        {
            {0, HID_KEY_F1, HID_KEY_F2, HID_KEY_F3, HID_KEY_F4, HID_KEY_F5, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}
        },
        // Right half
        {
            {0, HID_KEY_F6, HID_KEY_F7, HID_KEY_F8, HID_KEY_F9, HID_KEY_F10, HID_KEY_F11},
            {0, 0, 0, HID_KEY_ARROW_UP, 0, 0, HID_KEY_F12},
            {0, 0, HID_KEY_ARROW_LEFT, HID_KEY_ARROW_DOWN, HID_KEY_ARROW_RIGHT, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}
        }
    },
    // Layer 2 - Mouse and Macros
    {
        // Left half
        {
            {0, KEY_MACRO_0, KEY_MACRO_0+1, KEY_MACRO_0+2, 0, 0, 0},
            {0, KEY_MOUSE_LEFT, KEY_MOUSE_UP, KEY_MOUSE_RIGHT, 0, 0, 0},
            {0, KEY_MOUSE_LEFT_MOVE, KEY_MOUSE_DOWN, KEY_MOUSE_RIGHT_MOVE, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, KEY_AUTO_CLICK, 0, 0}
        },
        // Right half
        {
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}
        }
    },
    // Layer 3 - Custom
    {
        // Left half
        {
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}
        },
        // Right half
        {
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0}
        }
    }
};

void init_macros(void) {
    // Example macro 0: "Hello"
    macros[0].length = 5;
    macros[0].keys[0] = HID_KEY_H;
    macros[0].keys[1] = HID_KEY_E;
    macros[0].keys[2] = HID_KEY_L;
    macros[0].keys[3] = HID_KEY_L;
    macros[0].keys[4] = HID_KEY_O;
    
    // Example macro 1: Ctrl+C
    macros[1].length = 1;
    macros[1].keys[0] = HID_KEY_C | 0x80;  // High bit indicates Ctrl modifier
    
    // Add more macros as needed
}

void add_key_to_report(uint8_t keycode) {
    // Check if it's a modifier
    if (keycode >= HID_MOD_LEFT_CTRL && keycode <= HID_MOD_RIGHT_GUI) {
        kbd_report[0] |= keycode;
        return;
    }
    
    // Add to key slots (skip if already present)
    for (int i = 2; i < 8; i++) {
        if (kbd_report[i] == keycode) return;
    }
    
    // Find empty slot
    for (int i = 2; i < 8; i++) {
        if (kbd_report[i] == 0) {
            kbd_report[i] = keycode;
            return;
        }
    }
}

void remove_key_from_report(uint8_t keycode) {
    // Check if it's a modifier
    if (keycode >= HID_MOD_LEFT_CTRL && keycode <= HID_MOD_RIGHT_GUI) {
        kbd_report[0] &= ~keycode;
        return;
    }
    
    // Remove from key slots
    for (int i = 2; i < 8; i++) {
        if (kbd_report[i] == keycode) {
            kbd_report[i] = 0;
        }
    }
}

void send_keyboard_report(void) {
    if (tud_hid_ready()) {
        tud_hid_keyboard_report(1, kbd_report[0], &kbd_report[2]);
        report_changed = false;
    }
}

void send_mouse_report(void) {
    if (tud_hid_ready() && mouse_report_pending) {
        tud_hid_mouse_report(2, mouse_buttons, mouse_x, mouse_y, 0, 0);
        mouse_x = 0;
        mouse_y = 0;
        mouse_report_pending = false;
    }
}

void process_key_event(key_event_t* event) {
    uint8_t side = event->side;
    uint8_t row = event->row;
    uint8_t col = event->col;
    bool pressed = (event->type == 0);
    
    // Update key state
    key_state[side][row][col] = pressed;
    
    // Get keycode from current layer
    uint8_t keycode = keymap[current_layer][side][row][col];
    
    if (keycode == 0) return;  // No mapping
    
    // Handle special keys
    if (keycode >= KEY_LAYER_1 && keycode <= KEY_LAYER_3) {
        if (pressed) {
            current_layer = keycode - KEY_LAYER_1 + 1;
            printf("Layer: %d\n", current_layer);
        } else {
            current_layer = 0;  // Back to base layer
        }
        return;
    }
    
    // Handle macros
    if (keycode >= KEY_MACRO_0 && keycode < KEY_MACRO_0 + MAX_MACROS) {
        if (pressed) {
            active_macro = keycode - KEY_MACRO_0;
            macro_position = 0;
            last_macro_time = to_ms_since_boot(get_absolute_time());
            printf("Macro %d triggered\n", active_macro);
        }
        return;
    }
    
    // Handle mouse buttons
    if (keycode == KEY_MOUSE_LEFT) {
        if (pressed) mouse_buttons |= 0x01;
        else mouse_buttons &= ~0x01;
        mouse_report_pending = true;
        return;
    }
    if (keycode == KEY_MOUSE_RIGHT) {
        if (pressed) mouse_buttons |= 0x02;
        else mouse_buttons &= ~0x02;
        mouse_report_pending = true;
        return;
    }
    if (keycode == KEY_MOUSE_MIDDLE) {
        if (pressed) mouse_buttons |= 0x04;
        else mouse_buttons &= ~0x04;
        mouse_report_pending = true;
        return;
    }
    
    // Handle mouse movement
    if (keycode >= KEY_MOUSE_UP && keycode <= KEY_MOUSE_RIGHT_MOVE) {
        if (pressed) {
            switch(keycode) {
                case KEY_MOUSE_UP: mouse_y = -10; break;
                case KEY_MOUSE_DOWN: mouse_y = 10; break;
                case KEY_MOUSE_LEFT_MOVE: mouse_x = -10; break;
                case KEY_MOUSE_RIGHT_MOVE: mouse_x = 10; break;
            }
            mouse_report_pending = true;
        }
        return;
    }
    
    // Handle auto-click toggle
    if (keycode == KEY_AUTO_CLICK) {
        if (pressed) {
            auto_click_active = !auto_click_active;
            printf("Auto-click: %s\n", auto_click_active ? "ON" : "OFF");
        }
        return;
    }
    
    // Regular keyboard key
    if (pressed) {
        add_key_to_report(keycode);
    } else {
        remove_key_from_report(keycode);
    }
    report_changed = true;
}

void process_macro(void) {
    if (active_macro < 0) return;
    
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_macro_time < MACRO_KEY_DELAY) return;
    
    macro_t* macro = &macros[active_macro];
    
    if (macro_position >= macro->length) {
        // Macro complete
        active_macro = -1;
        macro_position = 0;
        return;
    }
    
    uint8_t key = macro->keys[macro_position];
    bool has_ctrl = key & 0x80;
    key &= 0x7F;
    
    // Press
    memset(kbd_report, 0, sizeof(kbd_report));
    if (has_ctrl) kbd_report[0] = HID_MOD_LEFT_CTRL;
    kbd_report[2] = key;
    send_keyboard_report();
    
    sleep_ms(20);
    
    // Release
    memset(kbd_report, 0, sizeof(kbd_report));
    send_keyboard_report();
    
    macro_position++;
    last_macro_time = now;
}

void process_auto_click(void) {
    if (!auto_click_active) return;
    
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_auto_click < auto_click_interval) return;
    
    // Click
    mouse_buttons |= 0x01;
    mouse_report_pending = true;
    send_mouse_report();
    
    sleep_ms(20);
    
    // Release
    mouse_buttons &= ~0x01;
    mouse_report_pending = true;
    send_mouse_report();
    
    last_auto_click = now;
}

// BLE GATT handlers
static hci_con_handle_t left_handle = HCI_CON_HANDLE_INVALID;
static hci_con_handle_t right_handle = HCI_CON_HANDLE_INVALID;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    
    if (packet_type != HCI_EVENT_PACKET) return;
    
    switch (hci_event_packet_get_type(packet)) {
        case GATT_EVENT_NOTIFICATION: {
            key_event_t event;
            uint16_t value_length = gatt_event_notification_get_value_length(packet);
            if (value_length == sizeof(key_event_t)) {
                memcpy(&event, gatt_event_notification_get_value(packet), sizeof(event));
                process_key_event(&event);
            }
            break;
        }
        
        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            hci_con_handle_t handle = hci_event_disconnection_complete_get_connection_handle(packet);
            if (handle == left_handle) {
                left_handle = HCI_CON_HANDLE_INVALID;
                printf("Left half disconnected\n");
            } else if (handle == right_handle) {
                right_handle = HCI_CON_HANDLE_INVALID;
                printf("Right half disconnected\n");
            }
            break;
        }
    }
}

// USB HID callbacks
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void) instance;
    (void) report;
    (void) len;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, 
                                uint8_t* buffer, uint16_t reqlen) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, 
                            uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

int main() {
    stdio_init_all();
    
    // Initialize USB
    tusb_init();
    
    // Initialize macros
    init_macros();
    
    // Initialize CYW43 for BLE
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43\n");
        return 1;
    }
    
    // Initialize BTstack
    l2cap_init();
    sm_init();
    gatt_client_init();
    
    // Register packet handler
    hci_event_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    
    // Start scanning for keyboard halves
    gap_set_scan_parameters(0, 0x0030, 0x0030);
    gap_start_scan();
    
    // Turn on Bluetooth
    hci_power_control(HCI_POWER_ON);
    
    printf("Dongle initialized\n");
    printf("Scanning for keyboard halves...\n");
    
    // Main loop
    while (true) {
        // Process USB
        tud_task();
        
        // Process macros
        process_macro();
        
        // Process auto-click
        process_auto_click();
        
        // Send reports if needed
        if (report_changed) {
            send_keyboard_report();
        }
        if (mouse_report_pending) {
            send_mouse_report();
        }
        
        sleep_ms(1);
    }
    
    return 0;
}
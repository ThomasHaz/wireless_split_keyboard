/**
 * Keymap Configuration
 * Customize your keyboard layout here
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdint.h>

// Keyboard dimensions
#define ROWS 5
#define COLS 7
#define SIDES 2
#define MAX_LAYERS 4

// HID keycodes
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
#define HID_KEY_MINUS 0x2D           // - and _
#define HID_KEY_EQUAL 0x2E           // = and +
#define HID_KEY_LEFTBRACE 0x2F       // [ and {
#define HID_KEY_RIGHTBRACE 0x30      // ] and }
#define HID_KEY_BACKSLASH 0x31       // \ and |
#define HID_KEY_HASHTILDE 0x32       // # and ~
#define HID_KEY_SEMICOLON 0x33       // ; and :
#define HID_KEY_APOSTROPHE 0x34      // ' and "
#define HID_KEY_GRAVE 0x35           // ` and ~
#define HID_KEY_COMMA 0x36           // , and <
#define HID_KEY_DOT 0x37             // . and >
#define HID_KEY_SLASH 0x38           // / and ?
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
#define HID_KEY_PRINT_SCREEN 0x46
#define HID_KEY_SCROLL_LOCK 0x47
#define HID_KEY_PAUSE 0x48
#define HID_KEY_INSERT 0x49
#define HID_KEY_HOME 0x4A
#define HID_KEY_PAGEUP 0x4B
#define HID_KEY_DELETE 0x4C
#define HID_KEY_END 0x4D
#define HID_KEY_PAGEDOWN 0x4E
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

// Convenience macro for empty keys
#define ___ 0

// Keymap - [layer][side][row][col]
// Customize this to match your preferred layout
static const uint8_t keymap[MAX_LAYERS][SIDES][ROWS][COLS] = {
    // Layer 0 - Base QWERTY
    {
        // Left half
        {
            {HID_KEY_ESC,        HID_KEY_1,       HID_KEY_2,       HID_KEY_3,       HID_KEY_4,       HID_KEY_5,       ___},
            {HID_KEY_TAB,        HID_KEY_Q,       HID_KEY_W,       HID_KEY_E,       HID_KEY_R,       HID_KEY_T,       ___},
            {HID_KEY_CAPS_LOCK,  HID_KEY_A,       HID_KEY_S,       HID_KEY_D,       HID_KEY_F,       HID_KEY_G,       ___},
            {HID_MOD_LEFT_SHIFT, HID_KEY_Z,       HID_KEY_X,       HID_KEY_C,       HID_KEY_V,       HID_KEY_B,       ___},
            {HID_MOD_LEFT_CTRL,  HID_MOD_LEFT_GUI,HID_MOD_LEFT_ALT,KEY_LAYER_1,     HID_KEY_SPACE,   ___,             ___}
        },
        // Right half
        {
            {___,                HID_KEY_6,       HID_KEY_7,       HID_KEY_8,            HID_KEY_9,           HID_KEY_0,           HID_KEY_BACKSPACE},
            {___,                HID_KEY_Y,       HID_KEY_U,       HID_KEY_I,            HID_KEY_O,           HID_KEY_P,           HID_KEY_LEFTBRACE},
            {___,                HID_KEY_H,       HID_KEY_J,       HID_KEY_K,            HID_KEY_L,           HID_KEY_SEMICOLON,   HID_KEY_APOSTROPHE},
            {___,                HID_KEY_N,       HID_KEY_M,       HID_KEY_COMMA,        HID_KEY_DOT,         HID_KEY_SLASH,       HID_MOD_RIGHT_SHIFT},
            {___,                ___,             HID_KEY_SPACE,   KEY_LAYER_2,          HID_MOD_RIGHT_ALT,   HID_MOD_RIGHT_CTRL,  ___}
        }
    },
    
    // Layer 1 - Function/Navigation
    {
        // Left half
        {
            {___,                HID_KEY_F1,      HID_KEY_F2,      HID_KEY_F3,      HID_KEY_F4,      HID_KEY_F5,      ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___}
        },
        // Right half
        {
            {___,                HID_KEY_F6,      HID_KEY_F7,      HID_KEY_F8,           HID_KEY_F9,          HID_KEY_F10,         HID_KEY_F11},
            {___,                HID_KEY_HOME,    HID_KEY_PAGEUP,  HID_KEY_ARROW_UP,     HID_KEY_PAGEDOWN,    HID_KEY_END,         HID_KEY_F12},
            {___,                ___,             HID_KEY_ARROW_LEFT, HID_KEY_ARROW_DOWN, HID_KEY_ARROW_RIGHT, ___,              HID_KEY_ENTER},
            {___,                ___,             ___,             ___,                  ___,                 ___,                 ___},
            {___,                ___,             ___,             ___,                  ___,                 ___,                 ___}
        }
    },
    
    // Layer 2 - Mouse and Macros
    {
        // Left half
        {
            {___,                KEY_MACRO_0,     KEY_MACRO_0+1,   KEY_MACRO_0+2,   ___,             ___,             ___},
            {___,                KEY_MOUSE_LEFT,  KEY_MOUSE_UP,    KEY_MOUSE_RIGHT, ___,             ___,             ___},
            {___,                KEY_MOUSE_LEFT_MOVE, KEY_MOUSE_DOWN, KEY_MOUSE_RIGHT_MOVE, ___,      ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             KEY_AUTO_CLICK,  ___,             ___}
        },
        // Right half
        {
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___}
        }
    },
    
    // Layer 3 - Custom (Numbers and Symbols)
    {
        // Left half
        {
            {HID_KEY_GRAVE,      HID_KEY_F1,      HID_KEY_F2,      HID_KEY_F3,      HID_KEY_F4,      HID_KEY_F5,      ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___}
        },
        // Right half
        {
            {___,                HID_KEY_F6,      HID_KEY_F7,      HID_KEY_F8,      HID_KEY_F9,      HID_KEY_F10,     HID_KEY_DELETE},
            {___,                HID_KEY_MINUS,   HID_KEY_EQUAL,   HID_KEY_LEFTBRACE, HID_KEY_RIGHTBRACE, HID_KEY_BACKSLASH, ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             ___,             ___,             ___,             ___},
            {___,                ___,             ___,             KEY_LAYER_3,     ___,             ___,             ___}
        }
    }
};

#endif // KEYMAP_H
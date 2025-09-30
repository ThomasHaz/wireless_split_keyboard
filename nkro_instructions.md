# N-Key Rollover (NKRO) Implementation Guide

## Current: 6-Key Rollover (6KRO)
Your keyboard currently supports 6 simultaneous key presses plus all modifiers. This is the standard USB HID boot protocol.

## Upgrading to Full NKRO

To enable full N-key rollover (all keys simultaneously), you need to change the HID report descriptor to use a bitmap instead of an array.

### Changes Required

#### 1. Update `usb_descriptors.c`

Replace the keyboard HID report descriptor with an NKRO bitmap version:

```c
// NKRO HID Report Descriptor
uint8_t const desc_hid_report[] = {
    // NKRO Keyboard Report (Report ID 1)
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    
    // Modifiers (8 bits)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0xE0,        //   Usage Minimum (Keyboard Left Control)
    0x29, 0xE7,        //   Usage Maximum (Keyboard Right GUI)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Reserved byte
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Constant)
    
    // Bitmap for keys 0x00-0xA4 (165 keys = 21 bytes)
    0x05, 0x07,        //   Usage Page (Keyboard/Keypad)
    0x19, 0x00,        //   Usage Minimum (0)
    0x29, 0xA4,        //   Usage Maximum (164)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0xA5,        //   Report Count (165)
    0x81, 0x02,        //   Input (Data, Variable, Absolute)
    
    // Padding to byte boundary (165 bits = 20 bytes + 5 bits, need 3 more bits)
    0x95, 0x03,        //   Report Count (3)
    0x75, 0x01,        //   Report Size (1)
    0x81, 0x01,        //   Input (Constant)
    
    0xC0,              // End Collection
    
    // Mouse Report (Report ID 2) - keep the same
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2))
};
```

#### 2. Update `dongle.c` - Change Report Structure

Replace the keyboard report with a bitmap:

```c
// NKRO Report: 1 byte modifiers + 1 byte reserved + 21 bytes bitmap = 23 bytes
static uint8_t kbd_report[23] = {0};  // Bitmap for NKRO
static bool report_changed = false;

void add_key_to_report(uint8_t keycode) {
    // Check if it's a modifier
    if (keycode >= HID_MOD_LEFT_CTRL && keycode <= HID_MOD_RIGHT_GUI) {
        kbd_report[0] |= keycode;
        return;
    }
    
    // For regular keys, set the bit in the bitmap
    // Byte position: 2 + (keycode / 8)
    // Bit position: keycode % 8
    if (keycode <= 0xA4) {
        uint8_t byte_pos = 2 + (keycode / 8);
        uint8_t bit_pos = keycode % 8;
        kbd_report[byte_pos] |= (1 << bit_pos);
    }
}

void remove_key_from_report(uint8_t keycode) {
    // Check if it's a modifier
    if (keycode >= HID_MOD_LEFT_CTRL && keycode <= HID_MOD_RIGHT_GUI) {
        kbd_report[0] &= ~keycode;
        return;
    }
    
    // For regular keys, clear the bit in the bitmap
    if (keycode <= 0xA4) {
        uint8_t byte_pos = 2 + (keycode / 8);
        uint8_t bit_pos = keycode % 8;
        kbd_report[byte_pos] &= ~(1 << bit_pos);
    }
}

void send_keyboard_report(void) {
    if (tud_hid_ready()) {
        // Send raw report (NKRO)
        tud_hid_report(1, kbd_report, sizeof(kbd_report));
        report_changed = false;
    }
}
```

## Trade-offs

### 6KRO (Current)
**Pros:**
- ✅ Works in BIOS/UEFI
- ✅ Universal compatibility
- ✅ Smaller report (8 bytes)
- ✅ Lower bandwidth

**Cons:**
- ❌ Only 6 simultaneous keys (plus modifiers)

### NKRO (Upgrade)
**Pros:**
- ✅ All keys can be pressed simultaneously
- ✅ Perfect for gaming/complex shortcuts
- ✅ No rollover limitations

**Cons:**
- ❌ Won't work in BIOS/UEFI
- ❌ May not work with some KVMs
- ❌ Larger report (23 bytes)
- ❌ Slightly higher latency

## Hybrid Solution (Best of Both Worlds)

You can implement **both** 6KRO and NKRO modes:

1. Create two HID report descriptors (IDs 1 and 3)
2. Use Report ID 1 for 6KRO (boot protocol)
3. Use Report ID 3 for NKRO (bitmap)
4. Add a key combo to toggle between modes

This gives you BIOS compatibility when needed and full NKRO during normal use.

## Do You Need NKRO?

For most users, **6KRO is sufficient**:
- 6 keys + all modifiers covers 99% of typing scenarios
- Most key combinations use 2-4 keys max
- Even complex gaming typically uses fewer than 6 keys

**You might want NKRO if:**
- Playing games that require 7+ simultaneous keys
- Using complex CAD/audio production shortcuts
- You want the "ultimate" keyboard experience

## Recommendation

For this keyboard project, I'd suggest **sticking with 6KRO** unless you have a specific need for NKRO. The current implementation is:
- More compatible
- Simpler to implement
- Works everywhere
- Sufficient for 99% of use cases

If you later decide you need NKRO, you can always upgrade by following the instructions above!
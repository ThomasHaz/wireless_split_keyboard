# Wireless Split Keyboard with Raspberry Pi Pico W

A fully wireless split keyboard using three Raspberry Pi Pico W boards with BLE wireless UART for low latency communication.

## Features

- **Wireless Split Design**: Two keyboard halves communicate wirelessly with a dongle
- **Low Latency**: Uses BLE UART for fast communication
- **USB Dongle**: No need to pair with computer - plug and play
- **4 Keymap Layers**: Switch between different layouts on the fly
- **Macro Support**: Record and playback key sequences
- **Mouse Control**: Full mouse movement and clicking via keyboard
- **Auto-clicker**: Toggle automatic mouse clicking
- **Customizable Layout**: Easy keymap editing

## Hardware Requirements

- 3x Raspberry Pi Pico W boards
- Keyboard switches (your choice: Cherry MX, Kailh, etc.)
- Diodes (1N4148) - one per key
- PCB or hand-wired matrix
- USB cable for dongle
- Batteries for keyboard halves (optional - can run on USB power during development)

## Wiring Guide

### Matrix Wiring (Both Halves)

Each keyboard half uses a standard matrix layout:

```
Rows (GPIO 2-6): Connect to switch rows via diodes
Columns (GPIO 7-13): Connect to switch columns
```

**Standard Matrix Connection:**
```
       Col0  Col1  Col2  ...
Row0 --|>----|>----|>----...
Row1 --|>----|>----|>----...
Row2 --|>----|>----|>----...
```

The diode cathode (marked line) goes toward the row. This prevents ghosting.

### Pin Assignments

**Default pins (customize in code):**
- Rows: GPIO 2, 3, 4, 5, 6
- Columns: GPIO 7, 8, 9, 10, 11, 12, 13

You can modify these in the `row_pins` and `col_pins` arrays in the source code.

## Software Setup

### Prerequisites

1. **Pico SDK**: Install the Raspberry Pi Pico SDK
   ```bash
   cd ~
   git clone https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   export PICO_SDK_PATH=~/pico-sdk
   ```

2. **Build Tools**: Install CMake and ARM GCC
   ```bash
   # Ubuntu/Debian
   sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
   
   # macOS
   brew install cmake
   brew install --cask gcc-arm-embedded
   ```

### Building the Firmware

1. **Clone or create project directory:**
   ```bash
   mkdir wireless_split_keyboard
   cd wireless_split_keyboard
   ```

2. **Copy all source files** (left_half.c, right_half.c, dongle.c, usb_descriptors.c, tusb_config.h, CMakeLists.txt)

3. **Get pico_sdk_import.cmake:**
   ```bash
   cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
   ```

4. **Build:**
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j4
   ```

This will create three `.uf2` files:
- `left_half.uf2`
- `right_half.uf2`
- `dongle.uf2`

### Flashing the Firmware

1. **Hold BOOTSEL button** on Pico W while connecting USB
2. Pico will appear as a USB drive
3. **Drag and drop** the appropriate `.uf2` file
4. Pico will reboot automatically

Flash each Pico W with its corresponding firmware:
- Left keyboard half → `left_half.uf2`
- Right keyboard half → `right_half.uf2`
- Dongle → `dongle.uf2`

## Usage

### First Time Setup

1. **Flash all three devices** with their respective firmware
2. **Power on both keyboard halves** (USB or battery)
3. **Connect dongle to computer** via USB
4. The dongle will automatically discover and connect to both halves
5. **Start typing!**

### Layer System

The keyboard supports 4 layers (0-3):

- **Layer 0 (Base)**: Standard QWERTY layout
- **Layer 1**: Function keys and navigation
- **Layer 2**: Mouse control and macros
- **Layer 3**: Custom (define your own)

**Layer Keys:**
- Hold `KEY_LAYER_1` to access Layer 1
- Hold `KEY_LAYER_2` to access Layer 2
- Release to return to Layer 0

### Mouse Control (Layer 2)

- **Mouse Movement**: Arrow key equivalents move cursor
- **Left Click**: Mapped to specific key
- **Right Click**: Mapped to specific key
- **Middle Click**: Mapped to specific key
- **Auto-click**: Toggle automatic left clicking

### Macros

**Pre-defined macros** are in the `init_macros()` function:
- Macro 0: Types "Hello"
- Macro 1: Sends Ctrl+C

**To add custom macros**, edit the `init_macros()` function in `dongle.c`:

```c
// Example: Add a macro that types your email
macros[2].length = 14;
macros[2].keys[0] = HID_KEY_U;
macros[2].keys[1] = HID_KEY_S;
macros[2].keys[2] = HID_KEY_E;
macros[2].keys[3] = HID_KEY_R;
// ... continue for "user@email.com"
```

For macros with modifiers, set the high bit:
```c
macros[3].keys[0] = HID_KEY_V | 0x80;  // Ctrl+V
```

## Customization

### Changing the Keymap

Edit the `keymap` array in `dongle.c`. The structure is:
```c
keymap[layer][side][row][col]
```

Where:
- `layer`: 0-3 (which layer)
- `side`: 0=left, 1=right
- `row`: 0-4 (physical row)
- `col`: 0-6 (physical column)

**Example:**
```c
// Set Layer 0, Right side, Row 2, Column 3 to 'K'
keymap[0][1][2][3] = HID_KEY_K;

// Set a modifier
keymap[0][0][4][0] = HID_MOD_LEFT_SHIFT;

// Set a layer switch
keymap[0][0][4][3] = KEY_LAYER_1;
```

### Available Key Codes

See the `#define HID_KEY_*` section in `dongle.c` for all available keys. Common ones:

- Letters: `HID_KEY_A` through `HID_KEY_Z`
- Numbers: `HID_KEY_1` through `HID_KEY_0`
- Modifiers: `HID_MOD_LEFT_CTRL`, `HID_MOD_LEFT_SHIFT`, etc.
- Special: `HID_KEY_ENTER`, `HID_KEY_ESC`, `HID_KEY_SPACE`, etc.
- Function: `HID_KEY_F1` through `HID_KEY_F12`
- Layers: `KEY_LAYER_1`, `KEY_LAYER_2`, `KEY_LAYER_3`
- Macros: `KEY_MACRO_0` through `KEY_MACRO_0+15`
- Mouse: `KEY_MOUSE_LEFT`, `KEY_MOUSE_RIGHT`, `KEY_MOUSE_UP`, etc.

### Adjusting Matrix Size

If your keyboard has a different number of rows/columns:

1. Change `ROWS` and `COLS` in all three source files
2. Update `row_pins` and `col_pins` arrays
3. Resize the `keymap` array accordingly
4. Rebuild firmware

### Power Optimization

For battery operation, you can add sleep modes:

```c
// In the main loop of keyboard halves
if (!connected) {
    sleep_ms(100);  // Sleep when not connected
}
```

Consider using deep sleep modes from the Pico SDK for extended battery life.

## Troubleshooting

### Keyboard Half Not Connecting

1. Check serial output (connect via USB and use minicom/screen)
2. Verify both halves are powered on
3. Check BLE initialization messages
4. Ensure dongle is scanning

### Keys Not Registering

1. Test matrix with multimeter - check for shorts
2. Verify diode orientation (cathode toward row)
3. Check pin assignments match your wiring
4. Monitor serial output for key press detection

### USB Not Recognized

1. Verify TinyUSB configuration
2. Check USB cable (must support data, not just power)
3. Try different USB port
4. Check dongle serial output for errors

### High Latency

1. BLE UART should give ~10-20ms latency
2. Reduce scan interval in main loops
3. Check for BLE interference
4. Ensure strong signal between devices

## Development Tips

### Debugging

All three devices output debug info via USB serial:

```bash
# Linux
sudo screen /dev/ttyACM0 115200

# macOS  
screen /dev/cu.usbmodem* 115200

# Windows
# Use PuTTY or similar
```

### Testing Individual Components

1. **Matrix Test**: Comment out BLE code, just print key presses
2. **BLE Test**: Send test packets without keyboard matrix
3. **USB Test**: Test HID reports with known values

### Power Consumption

Monitor current draw:
- Active scanning: ~30-50mA per half
- Idle: ~10-20mA per half
- Dongle: ~50-80mA (USB powered)

## Future Enhancements

Ideas for extending this project:

- **OLED Display**: Add status display to keyboard halves
- **RGB Backlighting**: Per-key RGB LED control
- **Split Communication**: Direct left-right communication for key combinations
- **Configuration**: Store keymap/settings in flash
- **Battery Monitoring**: Display battery level
- **Wireless Charging**: Add wireless charging coils
- **Encoders**: Add rotary encoder support
- **Haptic Feedback**: Add vibration motors

## License

This project is open source. Feel free to modify and distribute.

## Credits

Built with:
- Raspberry Pi Pico SDK
- BTstack for Bluetooth
- TinyUSB for USB HID

## Support

For issues or questions, check:
- Raspberry Pi Pico documentation
- BTstack documentation  
- QMK keyboard documentation (for keymap inspiration)
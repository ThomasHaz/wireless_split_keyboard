cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

# Set board type for Pico W
set(PICO_BOARD pico_w)

project(wireless_split_keyboard C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Verify we're building for Pico W
if (NOT PICO_CYW43_SUPPORTED)
    message(FATAL_ERROR "This project requires a Pico W board (PICO_BOARD=pico_w)")
endif()

# Common compiler flags
add_compile_options(-Wall)

# Add lwIP include path
include_directories(${CMAKE_CURRENT_LIST_DIR})

#
# Left Keyboard Half
#
add_executable(left_half
    left_half.c
    btstack_tlv_stub.c
)

target_link_libraries(left_half
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
)

pico_enable_stdio_usb(left_half 0)
pico_enable_stdio_uart(left_half 1)

pico_add_extra_outputs(left_half)

#
# Right Keyboard Half
#
add_executable(right_half
    right_half.c
    btstack_tlv_stub.c
)

target_link_libraries(right_half
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
)

pico_enable_stdio_usb(right_half 0)
pico_enable_stdio_uart(right_half 1)

pico_add_extra_outputs(right_half)

#
# Dongle
#
add_executable(dongle
    dongle.c
    usb_descriptors.c
    btstack_tlv_stub.c
)

target_link_libraries(dongle
    pico_stdlib
    pico_cyw43_arch_none
    pico_btstack_ble
    pico_btstack_cyw43
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(dongle 0)
pico_enable_stdio_uart(dongle 1)

pico_add_extra_outputs(dongle)

# Add compile definitions for TinyUSB
target_compile_definitions(dongle PRIVATE
    CFG_TUSB_CONFIG_FILE="tusb_config.h"
)
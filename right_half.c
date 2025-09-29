/**
 * Right Keyboard Half
 * Scans matrix and sends key events via wireless UART to dongle
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include "hardware/gpio.h"

// Matrix configuration - adjust to your keyboard layout
#define ROWS 5
#define COLS 7

// GPIO pins for matrix (example - adjust to your wiring)
const uint row_pins[ROWS] = {2, 3, 4, 5, 6};
const uint col_pins[COLS] = {7, 8, 9, 10, 11, 12, 13};

// Debounce timing
#define DEBOUNCE_MS 5

// Key state tracking
static bool key_state[ROWS][COLS] = {0};
static uint32_t last_change_time[ROWS][COLS] = {0};

// UART Service UUID (custom)
static const char* UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* UART_TX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

static uint16_t uart_tx_handle = 0;
static hci_con_handle_t connection_handle = HCI_CON_HANDLE_INVALID;
static bool connected = false;

// Packet structure for key events
typedef struct {
    uint8_t type;      // 0 = key press, 1 = key release
    uint8_t row;
    uint8_t col;
    uint8_t side;      // 0 = left, 1 = right
} key_event_t;

void init_matrix(void) {
    // Initialize row pins as outputs (high)
    for (int i = 0; i < ROWS; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_OUT);
        gpio_put(row_pins[i], 1);
    }
    
    // Initialize column pins as inputs with pull-up
    for (int i = 0; i < COLS; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_IN);
        gpio_pull_up(col_pins[i]);
    }
}

void send_key_event(uint8_t type, uint8_t row, uint8_t col) {
    if (!connected || uart_tx_handle == 0) return;
    
    key_event_t event = {
        .type = type,
        .row = row,
        .col = col,
        .side = 1  // right side
    };
    
    att_server_notify(connection_handle, uart_tx_handle, 
                     (uint8_t*)&event, sizeof(event));
}

void scan_matrix(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    
    for (int row = 0; row < ROWS; row++) {
        // Set current row low
        gpio_put(row_pins[row], 0);
        sleep_us(10);  // Small delay for signal to settle
        
        for (int col = 0; col < COLS; col++) {
            bool current = !gpio_get(col_pins[col]);  // Active low
            
            // Debounce
            if (current != key_state[row][col]) {
                if (now - last_change_time[row][col] > DEBOUNCE_MS) {
                    key_state[row][col] = current;
                    last_change_time[row][col] = now;
                    
                    // Send event
                    send_key_event(current ? 0 : 1, row, col);
                    
                    printf("Key %s: R%d C%d\n", 
                           current ? "pressed" : "released", row, col);
                }
            }
        }
        
        // Set row back high
        gpio_put(row_pins[row], 1);
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);
    
    if (packet_type != HCI_EVENT_PACKET) return;
    
    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            connected = false;
            connection_handle = HCI_CON_HANDLE_INVALID;
            printf("Disconnected\n");
            break;
            
        case ATT_EVENT_CONNECTED:
            connection_handle = att_event_connected_get_handle(packet);
            connected = true;
            printf("Connected\n");
            break;
            
        case ATT_EVENT_CAN_SEND_NOW:
            // Ready to send more data
            break;
    }
}

static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, 
                                   uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    UNUSED(con_handle);
    UNUSED(att_handle);
    UNUSED(offset);
    UNUSED(buffer);
    UNUSED(buffer_size);
    return 0;
}

static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, 
                              uint16_t transaction_mode, uint16_t offset, 
                              uint8_t *buffer, uint16_t buffer_size) {
    UNUSED(con_handle);
    UNUSED(att_handle);
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer);
    UNUSED(buffer_size);
    return 0;
}

int main() {
    stdio_init_all();
    
    // Initialize matrix
    init_matrix();
    
    // Initialize CYW43 for BLE
    if (cyw43_arch_init()) {
        printf("Failed to initialize\n");
        return 1;
    }
    
    // Initialize BTstack
    l2cap_init();
    sm_init();
    att_server_init(NULL, att_read_callback, att_write_callback);
    
    // Set device name
    gap_set_local_name("KB_Right");
    gap_discoverable_control(1);
    gap_set_bondable_mode(1);
    
    // Register packet handler
    hci_event_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
    
    // Start advertising
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr = {0};
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_enable(1);
    
    // Turn on Bluetooth
    hci_power_control(HCI_POWER_ON);
    
    printf("Right keyboard half initialized\n");
    
    // Main loop
    while (true) {
        // Scan matrix at ~1000Hz
        scan_matrix();
        sleep_us(1000);
    }
    
    return 0;
}
/**
 * Minimal TLV implementation stub for BTstack
 * Provides required link key database functions
 */

#include "btstack_run_loop.h"

// Minimal TLV context
typedef struct {
    int dummy;
} btstack_tlv_t;

static btstack_tlv_t tlv_instance;

// Required function that returns TLV instance for link key database
const btstack_tlv_t * btstack_link_key_db_tlv_get_instance(void) {
    return &tlv_instance;
}
//-----------------------------------------------------------------------------
// File: config.h
//-----------------------------------------------------------------------------
#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>
#include <Preferences.h>

//-----------------------------------------------------------------------------
typedef struct {
    // Device's MAC address
    uint8_t mac_address[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint32_t serial_baudrate = 115200;
    uint32_t serial_tx_package_size = 1024;
    uint32_t serial_ack_timeout = 1000;
} config_t;

extern config_t config;

//-----------------------------------------------------------------------------
void config_init();
void config_task();
void config_commit();
void config_set_reset_flag();

#endif
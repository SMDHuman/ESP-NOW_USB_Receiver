//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#ifndef SERIAL_COM_H
#define SERIAL_COM_H
#include <Arduino.h>
#include "esp_camera.h"
#include "config.h"

//-----------------------------------------------------------------------------
#define BAUDRATE config.serial_baudrate
#define S_MAX_PACKAGE config.serial_tx_package_size
#define ACK_TIMEOUT config.serial_ack_timeout

//-----------------------------------------------------------------------------
enum rx_package_type_e{
    WRITE_CONFIG,
    READ_CONFIG,
    RESET_CONFIG
};

//-----------------------------------------------------------------------------
void serial_init();
void serial_task();
void serial_send_slip_tag(String data);
void serial_send_slip(uint8_t* buf, size_t len);
void serial_send_slip(uint8_t data);
void serial_send_debug(String text);
void serial_send_debug(int number);
void serial_end_slip();

//-----------------------------------------------------------------------------
#endif

//-----------------------------------------------------------------------------
// ESP Now receiver module header file
//-----------------------------------------------------------------------------
#ifndef WIRELESS_RECEIVER_H_
#define WIRELESS_RECEIVER_H_

//-----------------------------------------------------------------------------
#include <Arduino.h>
#include <esp_now.h>

//-----------------------------------------------------------------------------
// Function declarations
esp_err_t wrx_init();
esp_err_t wrx_send(uint8_t *data, uint8_t len);
esp_err_t wrx_receive(uint8_t *data, uint8_t len);
uint32_t wrx_get_len();
uint8_t wrx_data_received();

#endif
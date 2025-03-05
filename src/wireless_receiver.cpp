//-----------------------------------------------------------------------------
#include <wireless_receiver.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>
#include <config.h>
#include <serial_com.h>
#include <utils.h>

//-----------------------------------------------------------------------------
// variables needed
//-----------------------------------------------------------------------------
static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len);
static uint32_t wrx_len = 0;
static uint8_t data_received = false;

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

// Initialize the ESP-NOW receiver
esp_err_t wrx_init() {
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    esp_err_t ret;
    // Initialize ESP-NOW
    ret = esp_now_init();
    if (ret != ESP_OK) {
        return ret;
    }
    // Register callback function
    ret = esp_now_register_recv_cb(OnDataRecv);
    if (ret != ESP_OK) {
        return ret;
    }
    // Write the mac address to config
    WiFi.macAddress(config.mac_address);
    config_commit();
    //...
    return ESP_OK;
}

//...
esp_err_t wrx_send(uint8_t *addrss ,uint8_t *data, uint8_t len) {
    return esp_now_send(addrss, data, len);
}

//...
esp_err_t wrx_receive(uint8_t *data, uint8_t len) {
    return ESP_OK;
}

// Get received data length
uint32_t wrx_get_len() {
    return wrx_len;
}

// Check if data has been received
uint8_t wrx_data_received() {
    if(data_received){
        data_received = false;
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

// Callback function to handle received data
static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
    data_received = true;
    wrx_len = len;
    //...
    send_slip_tag("ESP_NOW_CB");
    send_slip((uint8_t*)mac_addr, 6);
    send_slip((uint8_t*)data, len);
    end_slip();
}
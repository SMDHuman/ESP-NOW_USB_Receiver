//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "serial_com.h"
#include "slip_decoder.h"
#include "utils.h"
#include "config.h"
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "command_handler.h"

static uint32_t checksum = 0;
static uint32_t data_count = 0;
static uint8_t wait_ack = false;

//-----------------------------------------------------------------------------
// Initialize the serial communication with the specified baud rate
void serial_init(){
  Serial.begin(BAUDRATE);
}

//-----------------------------------------------------------------------------
// Handle serial communication tasks, including reading and processing commands
void serial_task(){
  if(Serial.available()){
    slip_push(Serial.read());
  }
  if(slip_is_ready()){
    // Handle the message
    uint8_t *package;
    size_t package_len;
    slip_get_package(&package, &package_len);
    CMD_parse(package, package_len);
    slip_reset();
  }
}
//-----------------------------------------------------------------------------
// Send a string of data using SLIP encoding. Also puts leading zero byte at the end
void serial_send_slip_tag(String data){
  serial_send_slip((uint8_t*)data.c_str(), data.length());
  serial_send_slip(0);
}

//-----------------------------------------------------------------------------
// Send a buffer of data using SLIP encoding
void serial_send_slip(uint8_t *buf, size_t len){
  for(size_t i = 0; i < len; i++){
    serial_send_slip(buf[i]);
  }
}

//-----------------------------------------------------------------------------
// Send a single byte of data using SLIP encoding
void serial_send_slip(uint8_t data){
  // Split the package with an ACK if it is too large. 
  if(data_count == S_MAX_PACKAGE){
    Serial.write(S_ESC); // ESC+END == ACK
    Serial.write(S_END);
    // Wait for the ACK before sending the next package
    uint32_t timeout = millis();
    while(Serial.read() != S_ESC_END){
      if(millis() - timeout > ACK_TIMEOUT){
        //send_debug("ACK_TIMEOUT");
        return;
      }
    }
    data_count = 0;
  }
  // Send the data
  checksum += data;
  if(data == S_END){
    Serial.write(S_ESC);
    Serial.write(S_ESC_END);
  }
  else if(data == S_ESC){
    Serial.write(S_ESC);
    Serial.write(S_ESC_ESC);
  }
  else{
    Serial.write(data);
  }
  data_count++;
}

//-----------------------------------------------------------------------------
// End the SLIP transmission by sending the checksum and end byte
void serial_end_slip(){
  convert32_u value{
    .number = checksum
  };
  serial_send_slip(value.div4, 4);
  Serial.write(S_END);
  checksum = 0;
  data_count = 0;
}

//-----------------------------------------------------------------------------
// Send a debug message as a string using SLIP encoding
void serial_send_debug(String text){
  serial_send_slip_tag("DEBUG_STR");
  for(size_t i = 0; i < text.length(); i++){
    serial_send_slip(text[i]);
  }
  serial_end_slip();
}
//-----------------------------------------------------------------------------
// Send a custom message as a string using SLIP encoding
void serial_send_custom(String tag, String text){
  serial_send_slip_tag(tag);
  for(size_t i = 0; i < text.length(); i++){
    serial_send_slip(text[i]);
  }
  serial_end_slip();
}
//-----------------------------------------------------------------------------
// Send a custom message as an char using SLIP encoding
void serial_send_custom(String tag, uint8_t number){
  serial_send_slip_tag(tag);
  serial_send_slip(number);
  serial_end_slip();
}
//-----------------------------------------------------------------------------
// Send a custom message as a buffer using SLIP encoding
void serial_send_custom(String tag, uint8_t *buf, size_t len){
  serial_send_slip_tag(tag);
  for(size_t i = 0; i < len; i++){
    serial_send_slip(buf[i]);
  }
  serial_end_slip();
}
//-----------------------------------------------------------------------------
// Send a debug message as an integer using SLIP encoding
void serial_send_debug(int number){
  serial_send_debug(String(number));
}
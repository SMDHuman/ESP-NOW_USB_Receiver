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

static uint32_t checksum = 0;
static uint32_t data_count = 0;
static uint8_t wait_ack = false;
static esp_now_peer_info_t peer_info;

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
    // Extract the message tag
    String msg_tag = "";
    uint8_t msg_start = 0;
    while((slip_package_buffer[msg_start] != 0) & (msg_start < slip_buffer_index)){
      msg_tag += (char)slip_package_buffer[msg_start++];
    }
    msg_start++;

    uint32_t package_size = slip_buffer_index - msg_start;
    uint8_t *msg_data = &slip_package_buffer[msg_start];
    // PING - Respond with PONG
    if(msg_tag == "PING"){
      send_debug("PONG");
    }
    // WRITE_CONFIG - Write the configuration to the memory [config]
    else if(msg_tag == "WRITE_CONFIG"){
      memcpy(&config, &slip_package_buffer[msg_start], sizeof(config));
      config_commit();
    }
    // READ_CONFIG - Send the configuration to the sender
    else if(msg_tag == "READ_CONFIG"){
      send_slip_tag("CONFIG");
      uint8_t config_package[sizeof(config_t)];
      memcpy(config_package, &config, sizeof(config_t));
      send_slip(config_package, sizeof(config_t));
      end_slip();
    }
    // RESET_CONFIG - Reset the configuration
    else if(msg_tag == "RESET_CONFIG"){
      config_set_reset_flag();
    }
    // ADD_PEER - Add a peer to the list [peer_addr]
    else if(msg_tag == "ADD_PEER"){
      memcpy(peer_info.peer_addr, &slip_package_buffer[msg_start], 6);
      peer_info.channel = 0; 
      peer_info.encrypt = false;
      esp_err_t res = esp_now_add_peer(&peer_info);
      if(res != ESP_OK){
        send_debug("!PEER_EXISTS!");
      }
    }
    // REMOVE_PEER - Remove a peer from the list [peer_addr]
    else if(msg_tag == "REMOVE_PEER"){
      esp_err_t res = esp_now_del_peer(&slip_package_buffer[msg_start]);
      if(res != ESP_OK){
        send_debug("!PEER_NOT_FOUND!");
      }
    }
    // SEND - Send data to a peer [peer_addr, data]
    else if(msg_tag == "SEND"){
      uint8_t peer_addr[6];
      memcpy(peer_addr, &slip_package_buffer[msg_start], 6);
      esp_err_t res = esp_now_send(peer_addr, 
                                   &slip_package_buffer[msg_start+6], 
                                   slip_buffer_index-msg_start-6);
      if(res != ESP_OK){
        send_debug("!SEND_FAIL!");
      }
    }
    slip_reset();
  }
}
//-----------------------------------------------------------------------------
// Send a string of data using SLIP encoding. Also puts leading zero byte at the end
void send_slip_tag(String data){
  send_slip((uint8_t*)data.c_str(), data.length());
  send_slip(0);
}

//-----------------------------------------------------------------------------
// Send a buffer of data using SLIP encoding
void send_slip(uint8_t *buf, size_t len){
  for(size_t i = 0; i < len; i++){
    send_slip(buf[i]);
  }
}

//-----------------------------------------------------------------------------
// Send a single byte of data using SLIP encoding
void send_slip(uint8_t data){
  // Split the package with an ACK if it is too large. Wait for the ACK before sending the next package
  if(data_count == S_MAX_PACKAGE){
    Serial.write(S_ESC); // ESC+END == ACK
    Serial.write(S_END);
    // Wait for ACK
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
void end_slip(){
  convert32_u value{
    .number = checksum
  };
  send_slip(value.div4, 4);
  Serial.write(S_END);
  checksum = 0;
  data_count = 0;
}

//-----------------------------------------------------------------------------
// Send a debug message as a string using SLIP encoding
void send_debug(String text){
  send_slip_tag("DEBUG_STR");
  for(size_t i = 0; i < text.length(); i++){
    send_slip(text[i]);
  }
  end_slip();
}

//-----------------------------------------------------------------------------
// Send a debug message as an integer using SLIP encoding
void send_debug(int number){
  send_debug(String(number));
}
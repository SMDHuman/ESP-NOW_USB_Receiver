//-----------------------------------------------------------------------------
// File: command_handler.cpp
//-----------------------------------------------------------------------------
#include "command_handler.h"
#include <Arduino.h>
#include <esp_now.h>
#include "serial_com.h"

//-----------------------------------------------------------------------------
static esp_now_peer_info_t peer_info;

//-----------------------------------------------------------------------------
void CMD_init(){
}

//-----------------------------------------------------------------------------
void CMD_task(){
}
//-----------------------------------------------------------------------------
uint8_t CMD_parse(uint8_t *msg_data, uint32_t len){
  // Extract the message tag
  String msg_tag = "";
  uint8_t msg_start = 0;
  while((msg_data[msg_start] != 0) & (msg_start < len)){
    msg_tag += (char)msg_data[msg_start++];
  } msg_start++;
  uint32_t package_size = len - msg_start;
  uint8_t *payload = &msg_data[msg_start];
  // PING - Respond with PONG
  if(msg_tag == "PING"){
    serial_send_custom("PING_CB", "PONG");
  }
  // WRITE_CONFIG - Write the configuration to the memory [config]
  else if(msg_tag == "WRITE_CONFIG"){
    memcpy(&config, payload, sizeof(config));
    config_commit();
  }
  // READ_CONFIG - Send the configuration to the Serial.write
  else if(msg_tag == "READ_CONFIG"){
    serial_send_slip_tag("CONFIG");
    uint8_t config_package[sizeof(config_t)];
    memcpy(config_package, &config, sizeof(config_t));
    serial_send_slip(config_package, sizeof(config_t));
    serial_end_slip();
  }
  // RESET_CONFIG - Reset the configuration
  else if(msg_tag == "RESET_CONFIG"){
    config_set_reset_flag();
  }
  // ADD_PEER - Add a peer to the list [peer_addr]
  else if(msg_tag == "ADD_PEER"){
    memcpy(peer_info.peer_addr, payload, 6);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    esp_err_t res = esp_now_add_peer(&peer_info);
    if(res != ESP_OK){
      serial_send_debug("!PEER_EXISTS!");
    }
  }
  // REMOVE_PEER - Remove a peer from the list [peer_addr]
  else if(msg_tag == "REMOVE_PEER"){
    esp_err_t res = esp_now_del_peer(payload);
    if(res != ESP_OK){
      serial_send_debug("!PEER_NOT_FOUND!");
    }
  }
  // SEND - Send data to a peer [peer_addr, data]
  else if(msg_tag == "SEND"){
    uint8_t peer_addr[6];
    memcpy(peer_addr, payload, 6);
    esp_err_t res = esp_now_send(peer_addr,
                                 &payload[6],
                                 package_size-6);
    if(res != ESP_OK){
      serial_send_debug("!SEND_FAIL!");
    }
  }
  return 0;
}
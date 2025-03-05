#include <Arduino.h>
#include "config.h"
#include "wireless_receiver.h"
#include "serial_com.h"
//-----------------------------------------------------------------------------
void led_task();

//-----------------------------------------------------------------------------
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize the modules
  config_init();
  wrx_init();
  serial_init();
  //...
  config_set_reset_flag();
}

//-----------------------------------------------------------------------------
void loop() {
  serial_task();
  config_task();
  led_task();
  delay(1);
}

//-----------------------------------------------------------------------------
void led_task(){
  static uint64_t led_opened_at = millis();
  static uint8_t led_opened = false;
  if(millis() - led_opened_at > 50){
    digitalWrite(LED_BUILTIN, 0);
    led_opened = false;
  }
  if((wrx_data_received() | digitalRead(LED_BUILTIN))& !led_opened){
    led_opened_at = millis();
    digitalWrite(LED_BUILTIN, 1);
    led_opened = true;
  }
}

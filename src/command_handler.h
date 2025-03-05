#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>

void CMD_init();
void CMD_task();
uint8_t CMD_parse(uint8_t *msg_data, uint32_t package_size);

#endif
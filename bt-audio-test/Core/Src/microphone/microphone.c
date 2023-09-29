/*
 * microphone.c
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */
#include "stm32wbxx_hal.h"

void microphone_init(void) {
}

void microphone_start(void) {

}

void microphone_stop(void) {

}

void microphone_get(uint32_t * buffer, size_t count) {
  buffer[0] = buffer[0] + count + 1;
}

/*
 * microphone.h
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */

#ifndef SRC_MICROPHONE_MICROPHONE_H_
#define SRC_MICROPHONE_MICROPHONE_H_

void microphone_init(void);

void microphone_start(void);

void microphone_stop(void);

void microphone_get(uint32_t * buffer, size_t count);

#endif /* SRC_MICROPHONE_MICROPHONE_H_ */

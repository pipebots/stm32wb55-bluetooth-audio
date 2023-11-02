/*
 * microphone.h
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */

#ifndef SRC_MICROPHONE_MICROPHONE_H_
#define SRC_MICROPHONE_MICROPHONE_H_

#include <stddef.h>

/**
 * @file A microphone driver that writes a file to the QSPI Flash chip.
 */

/**
 * @brief Start recording of microphone data to the given file.
 * @param filename Name of the file to write to.
 * @param hAudioInSai Handle for the SAI block.
 * @return 0 for success, -1 for failure.
 */
int microphone_open(const char *filename, SAI_HandleTypeDef *hAudioInSai);

/**
 * @brief Returns file size in bytes.
 * @return Size of file in bytes.
 */
size_t microphone_get_file_size();

/**
 * @brief Close any open file.
 */
void microphone_close();

#endif /* SRC_MICROPHONE_MICROPHONE_H_ */

/*
 * microphone.h
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */

#ifndef SRC_MICROPHONE_MICROPHONE_H_
#define SRC_MICROPHONE_MICROPHONE_H_

/**
 * @file A microphone driver that writes a file to the QSPI Flash chip.
 */

/**
 * @brief Start recording of microphone data to the given file.
 * @param filename Name of the file to write to.
 * @return 0 for success, -1 for failure.
 */
int microphone_open(const char * filename);

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

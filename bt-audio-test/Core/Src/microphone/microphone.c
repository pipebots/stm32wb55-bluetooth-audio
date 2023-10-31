/*
 * microphone.c
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */
#include <cmsis_os.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "stm32wbxx_hal.h"

#include "cca02m2_audio.h"

// Thread variables.
static osThreadId_t m_thread_handle;
static const osThreadAttr_t m_thread_attributes = { .name = "Microphone",
    .priority = (osPriority_t) osPriorityNormal,
    // 4096 needed for printf and float.
    .stack_size = 1024 * 4 };

// File variables.
static const char *m_filename = NULL;
static int32_t m_file_size_bytes = 0;

// Audio variables.
static uint16_t m_pdm_buffer[((((AUDIO_IN_CHANNELS * AUDIO_IN_SAMPLING_FREQUENCY)
    / 1000) * MAX_DECIMATION_FACTOR) / 16) *
N_MS_PER_INTERRUPT];
static uint16_t m_pcm_buffer[((AUDIO_IN_CHANNELS * AUDIO_IN_SAMPLING_FREQUENCY)
    / 1000) * N_MS_PER_INTERRUPT];
static CCA02M2_AUDIO_Init_t m_mic_params;

/**
 * @brief  User function that is called when 1 ms of PDM data is available.
 *       In this application only PDM to PCM conversion and USB streaming
 *                  is performed.
 *       User can add his own code here to perform some DSP or audio analysis.
 * @param  none
 * @retval None
 */
void AudioProcess(void) {
  if (CCA02M2_AUDIO_IN_PDMToPCM(CCA02M2_AUDIO_INSTANCE,
                                (uint16_t*) m_pdm_buffer,
                                m_pcm_buffer) != BSP_ERROR_NONE) {
    Error_Handler();
  }
  // TODO Do something useful with the data.
  printf("%s: buffer data: 0x%04X, 0x%04X, 0x%04X, 0x%04X\n", __func__,
         (unsigned int) m_pcm_buffer[0], (unsigned int) m_pcm_buffer[1],
         (unsigned int) m_pcm_buffer[2], (unsigned int) m_pcm_buffer[3]);
}

/**
 * @brief  Half Transfer user callback, called by BSP functions.
 * @param  None
 * @retval None
 */
void CCA02M2_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance) {
  UNUSED(Instance);
  AudioProcess();
  }

/**
 * @brief  Transfer Complete user callback, called by BSP functions.
 * @param  None
 * @retval None
 */
void CCA02M2_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance) {
  UNUSED(Instance);
  AudioProcess();
}

static int open_file(const char *filename) {
  int result = -1;
  printf("MIC: Opening file %s\n", filename);
  m_filename = filename;
  // Do some stuff to open file.
  m_file_size_bytes = 0;
  // TODO Faked result for now.
  result = 0;
  // Check result.
  if (result != 0) {
    printf("MIC: ERROR: Could not open file %s/n", filename);
  }
  return result;
}

static void receive_data(void *parameter) {
  (void) parameter;
  m_mic_params.BitsPerSample = 16;
  m_mic_params.ChannelsNbr = 0;
  m_mic_params.Device = AUDIO_IN_DIGITAL_MIC;
  m_mic_params.SampleRate = 16000;
  m_mic_params.Volume = AUDIO_VOLUME_INPUT;

  if (CCA02M2_AUDIO_IN_Init(CCA02M2_AUDIO_INSTANCE,
                            &m_mic_params) != BSP_ERROR_NONE) {
    Error_Handler();
  }

  printf("MIC: Starting receive loop\n");
  while (1) {
    ++m_file_size_bytes;
    // Get data from queue and write to file.
//    while((m_file_size_bytes % 10) != 0) {
//    }
    printf("MIC: bytes: %ld\n", m_file_size_bytes);
    osDelay(100);
  }
}

static void microphone_power(bool enable) {
  GPIO_PinState new_state = enable ? GPIO_PIN_RESET : GPIO_PIN_SET;
  HAL_GPIO_WritePin(MIC_PWR_GPIO_Port, MIC_PWR_Pin, new_state);
  printf("MIC: %s: %d\n", __func__, new_state);
}

static void close_file(void) {
  printf("MIC: Closed file %s\n", m_filename);
  m_filename = 0;
}

/* API Functions */

int microphone_open(const char *filename) {
  int result = open_file(filename);
  if (result == 0) {
    // Power up mic.
    microphone_power(true);
    // Start rx thread.
    m_thread_handle = osThreadNew(receive_data, NULL, &m_thread_attributes);
  }
  return result;
}

size_t microphone_get_file_size() {
  static size_t file_size = 0;
  ++file_size;
  return file_size;
}

void microphone_close() {
  close_file();
  // Power down mic.
  microphone_power(false);
}


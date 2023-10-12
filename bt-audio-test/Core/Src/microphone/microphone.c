/*
 * microphone.c
 *
 *  Created on: Sep 29, 2023
 *      Author: real
 */
#include <cmsis_os.h>
#include <stdbool.h>
#include <stdio.h>

#include "main.h"
#include "stm32wbxx_hal.h"

static const char * m_filename = NULL;

// Thread variables.
static osThreadId_t m_thread_handle;
static const osThreadAttr_t m_thread_attributes = {
    .name = "Microphone",
    .priority = (osPriority_t)osPriorityNormal,
    // 4096 needed for printf and float.
    .stack_size = 1024 * 4
};
static int32_t m_file_size_bytes = 0;

//void microphone_init(void) {
//  /*
//  void stm_serial_init(UART_HandleTypeDef * uart_handle)
//  {
//    if (uart_handle) {
//      m_uart_handle = uart_handle;
//      HAL_UARTEx_ReceiveToIdle_DMA(
//        m_uart_handle, (uint8_t *)m_rx_buffer,
//        RX_BUFFER_SIZE);
//      __HAL_DMA_DISABLE_IT(m_uart_handle->hdmarx, DMA_IT_HT);
//      m_rx_message_queue =
//        osMessageQueueNew(m_msg_queue_len, IPC_ESP_MAX_MESSAGE_LENGTH, NULL);
//    }
//  }
//  */
//
//}
//
//void microphone_start(void) {
//  /*
//  HAL_StatusTypeDef HAL_SAI_Receive_DMA(SAI_HandleTypeDef *hsai, uint8_t *pData, uint16_t Size);
//  HAL_StatusTypeDef HAL_SAI_DMAPause(SAI_HandleTypeDef *hsai);
//  HAL_StatusTypeDef HAL_SAI_DMAResume(SAI_HandleTypeDef *hsai);
//  HAL_StatusTypeDef HAL_SAI_DMAStop(SAI_HandleTypeDef *hsai);
//  */
//}
//
//size_t microphone_get(uint32_t * buffer, size_t buffer_size) {
//  size_t count = 0;
//  // Bodge.
//  buffer[0] = buffer[0] + count + 1;
//  count = 1;
//  // Should be something like this.
//  // Copy audio words from internal buffer to given buffer until RX buffer
//  // empty or given buffer is full.
//  return count;
//}


static int open_file(const char * filename) {
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

int microphone_open(const char * filename) {
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


/*
 * companion_audio.h
 *
 *  Created on: Oct 12, 2023
 *      Author: real
 */

/* Based on code from this example: https://github.com/STMicroelectronics/x-cube-memsmic1/blob/main/Drivers/BSP/CCA02M2/cca02m2_audio.h */

#ifndef SRC_MICROPHONE_COMPANION_AUDIO_H_
#define SRC_MICROPHONE_COMPANION_AUDIO_H_

#include <stdint.h>

/* Common BSP Error codes */
#define BSP_ERROR_NONE                   (0)
#define BSP_ERROR_NO_INIT                (-1)
#define BSP_ERROR_WRONG_PARAM            (-2)
#define BSP_ERROR_BUSY                   (-3)
#define BSP_ERROR_PERIPH_FAILURE         (-4)
#define BSP_ERROR_COMPONENT_FAILURE      (-5)
#define BSP_ERROR_UNKNOWN_FAILURE        (-6)
#define BSP_ERROR_UNKNOWN_COMPONENT      (-7)
#define BSP_ERROR_BUS_FAILURE            (-8)
#define BSP_ERROR_CLOCK_FAILURE          (-9)
#define BSP_ERROR_MSP_FAILURE            (-10)

/* Audio In devices */
#ifndef AUDIO_IN_CHANNELS
#define AUDIO_IN_CHANNELS (1)
#endif

#ifndef AUDIO_IN_SAMPLING_FREQUENCY
#define AUDIO_IN_SAMPLING_FREQUENCY (16000)
#endif

/* Number of millisecond of audio at each DMA interrupt */
#ifndef N_MS_PER_INTERRUPT
#define N_MS_PER_INTERRUPT               (1U)
#endif

#ifndef COMPANION_AUDIO_INSTANCE
#define COMPANION_AUDIO_INSTANCE (0U)
#endif

#if (AUDIO_IN_SAMPLING_FREQUENCY == 8000)
#define MAX_DECIMATION_FACTOR 160
#else
#define MAX_DECIMATION_FACTOR 128
#endif

#ifndef AUDIO_VOLUME_INPUT
#define AUDIO_VOLUME_INPUT              (64U)
#endif

#define AUDIO_IN_DIGITAL_MIC1      0x10U
#define AUDIO_IN_DIGITAL_MIC2      0x20U
#define AUDIO_IN_DIGITAL_MIC3      0x40U
#define AUDIO_IN_DIGITAL_MIC4      0x80U
#define AUDIO_IN_DIGITAL_MIC_LAST  AUDIO_IN_DIGITAL_MIC4
// FIXME There is only one mic!
#define AUDIO_IN_DIGITAL_MIC       (AUDIO_IN_DIGITAL_MIC1 | AUDIO_IN_DIGITAL_MIC2 | AUDIO_IN_DIGITAL_MIC3 | AUDIO_IN_DIGITAL_MIC4)



typedef struct {
  uint32_t Device;
  uint32_t SampleRate;
  uint32_t BitsPerSample;
  uint32_t ChannelsNbr;
  uint32_t Volume;
} COMPANION_AUDIO_Init_t;

int32_t COMPANION_AUDIO_Init(uint32_t Instance,
                             COMPANION_AUDIO_Init_t *AudioInit);

int32_t COMPANION_AUDIO_Record(uint32_t Instance, uint8_t *pBuf,
                               uint32_t NbrOfBytes);

int32_t COMPANION_AUDIO_PDMToPCM(uint32_t Instance, uint16_t *PDMBuf,
                                 uint16_t *PCMBuf);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function should be implemented by the user application.
 It is called into this driver when the current buffer is filled to prepare the next
 buffer pointer and its size. */
void COMPANION_AUDIO_TransferComplete_CallBack(uint32_t Instance);
void COMPANION_AUDIO_HalfTransfer_CallBack(uint32_t Instance);

#endif /* SRC_MICROPHONE_COMPANION_AUDIO_H_ */

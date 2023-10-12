/*
 * companion_audio.c
 *
 *  Created on: Oct 12, 2023
 *      Author: real
 */
/* Based on code from this example: https://github.com/STMicroelectronics/x-cube-memsmic1/blob/main/Drivers/BSP/CCA02M2/cca02m2_audio.c */

#include "companion_audio.h"

#include <stdint.h>
#include <pdm2pcm_glo.h>

#ifndef ENABLE_HIGH_PERFORMANCE_MODE
#define ENABLE_HIGH_PERFORMANCE_MODE 0U
#endif

#if (ENABLE_HIGH_PERFORMANCE_MODE == 1U)
#define PDM_FREQ_16K 1536
#endif

#ifndef PDM_FREQ_16K
#define PDM_FREQ_16K 1280
#endif


/* Audio In states */
#define AUDIO_IN_STATE_RESET               0U
#define AUDIO_IN_STATE_RECORDING           1U
#define AUDIO_IN_STATE_STOP                2U
#define AUDIO_IN_STATE_PAUSE               3U

/* Audio In instances number:
 Instance 0 is SAI-I2S / SPI path
 Instance 1 is DFSDM path
 Instance 2 is PDM path
 */
#define AUDIO_IN_INSTANCES_NBR             3U

/* AUDIO FREQUENCY */
#ifndef AUDIO_FREQUENCY_192K
#define AUDIO_FREQUENCY_192K     (uint32_t)192000U
#endif
#ifndef AUDIO_FREQUENCY_176K
#define AUDIO_FREQUENCY_176K     (uint32_t)176400U
#endif
#ifndef AUDIO_FREQUENCY_96K
#define AUDIO_FREQUENCY_96K       (uint32_t)96000U
#endif
#ifndef AUDIO_FREQUENCY_88K
#define AUDIO_FREQUENCY_88K       (uint32_t)88200U
#endif
#ifndef AUDIO_FREQUENCY_48K
#define AUDIO_FREQUENCY_48K       (uint32_t)48000U
#endif
#ifndef AUDIO_FREQUENCY_44K
#define AUDIO_FREQUENCY_44K       (uint32_t)44100U
#endif
#ifndef AUDIO_FREQUENCY_32K
#define AUDIO_FREQUENCY_32K       (uint32_t)32000U
#endif
#ifndef AUDIO_FREQUENCY_22K
#define AUDIO_FREQUENCY_22K       (uint32_t)22050U
#endif
#ifndef AUDIO_FREQUENCY_16K
#define AUDIO_FREQUENCY_16K       (uint32_t)16000U
#endif
#ifndef AUDIO_FREQUENCY_11K
#define AUDIO_FREQUENCY_11K       (uint32_t)11025U
#endif
#ifndef AUDIO_FREQUENCY_8K
#define AUDIO_FREQUENCY_8K         (uint32_t)8000U
#endif

/* AUDIO RESOLUTION */
#ifndef AUDIO_RESOLUTION_16b
#define AUDIO_RESOLUTION_16b                16U
#endif
#ifndef AUDIO_RESOLUTION_24b
#define AUDIO_RESOLUTION_24b                24U
#endif
#ifndef AUDIO_RESOLUTION_32b
#define AUDIO_RESOLUTION_32b                32U
#endif

#ifndef AUDIO_IN_BIT_DEPTH
#define AUDIO_IN_BIT_DEPTH AUDIO_RESOLUTION_16b
#endif



typedef struct {
  int32_t Z;
  int32_t oldOut;
  int32_t oldIn;
} HP_FilterState_TypeDef;

typedef struct {
  uint32_t Instance; /* Audio IN instance              */
  uint32_t Device; /* Audio IN device to be used     */
  uint32_t SampleRate; /* Audio IN Sample rate           */
  uint32_t BitsPerSample; /* Audio IN Sample resolution     */
  uint32_t ChannelsNbr; /* Audio IN number of channel     */
  uint16_t *pBuff; /* Audio IN record buffer         */
  uint8_t **pMultiBuff; /* Audio IN multi-buffer          */
  uint32_t Size; /* Audio IN record buffer size    */
  uint32_t Volume; /* Audio IN volume                */
  uint32_t State; /* Audio IN State                 */
  uint32_t IsMultiBuff; /* Audio IN multi-buffer usage    */
  uint32_t IsMspCallbacksValid; /* Is Msp Callbacks registred     */
  HP_FilterState_TypeDef HP_Filters[4]; /*!< HP filter state for each channel*/
  uint32_t DecimationFactor;
} AUDIO_IN_Ctx_t;

/* Recording context */
AUDIO_IN_Ctx_t AudioInCtx[AUDIO_IN_INSTANCES_NBR] = { 0 };

int32_t COMPANION_AUDIO_Init(uint32_t Instance,
                             COMPANION_AUDIO_Init_t *AudioInit) {
  if (Instance >= AUDIO_IN_INSTANCES_NBR) {
    return BSP_ERROR_WRONG_PARAM;
  } else {
    /* Store the audio record context */
    AudioInCtx[Instance].Device = AudioInit->Device;
    AudioInCtx[Instance].ChannelsNbr = AudioInit->ChannelsNbr;
    AudioInCtx[Instance].SampleRate = AudioInit->SampleRate;
    AudioInCtx[Instance].BitsPerSample = AudioInit->BitsPerSample;
    AudioInCtx[Instance].Volume = AudioInit->Volume;
    AudioInCtx[Instance].State = AUDIO_IN_STATE_RESET;

    if (Instance == 0U) {
      uint32_t PDM_Clock_Freq;

      switch (AudioInit->SampleRate) {
        case AUDIO_FREQUENCY_8K:
          PDM_Clock_Freq = 1280;
          break;

        case AUDIO_FREQUENCY_16K:
          PDM_Clock_Freq = PDM_FREQ_16K;
          break;

        case AUDIO_FREQUENCY_32K:
          PDM_Clock_Freq = 2048;
          break;

        case AUDIO_FREQUENCY_48K:
          PDM_Clock_Freq = 3072;
          break;

        default:
          PDM_Clock_Freq = 0;
          break;
      }

      if (PDM_Clock_Freq == 0U) {
        return BSP_ERROR_WRONG_PARAM;
      }

      AudioInCtx[Instance].DecimationFactor = (PDM_Clock_Freq * 1000U)
          / AudioInit->SampleRate;
      /* Double buffer for 1 microphone */
      AudioInCtx[Instance].Size = (PDM_Clock_Freq / 8U) * 2U
          * N_MS_PER_INTERRUPT;

      // Initialise SAI device.
      if (AudioInCtx[Instance].ChannelsNbr == 1U) {
        AudioInCtx[Instance].Size *= 2U;
      }

      /* Initialize SAI */
      __HAL_SAI_RESET_HANDLE_STATE(&hAudioInSai);

      /* PLL clock is set depending by the AudioFreq */
      if (MX_SAI_ClockConfig(&hAudioInSai, PDM_Clock_Freq) != HAL_OK) {
        return BSP_ERROR_CLOCK_FAILURE;
      }

      if (HAL_SAI_GetState(&hAudioInSai) == HAL_SAI_STATE_RESET) {
        SAI_MspInit(&hAudioInSai);
      }

      hAudioInSai.Instance = AUDIO_IN_SAI_INSTANCE;
      __HAL_SAI_DISABLE(&hAudioInSai);

      hAudioInSai.Init.Protocol = SAI_FREE_PROTOCOL;
      hAudioInSai.Init.AudioMode = SAI_MODEMASTER_RX;
      hAudioInSai.Init.DataSize = SAI_DATASIZE_16;
      hAudioInSai.Init.FirstBit = SAI_FIRSTBIT_MSB;
      hAudioInSai.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
      hAudioInSai.Init.Synchro = SAI_ASYNCHRONOUS;
      hAudioInSai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
      hAudioInSai.Init.NoDivider = SAI_MASTERDIVIDER_DISABLE;
      hAudioInSai.Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
      hAudioInSai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
      hAudioInSai.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
      hAudioInSai.Init.MonoStereoMode = SAI_STEREOMODE;
      hAudioInSai.Init.CompandingMode = SAI_NOCOMPANDING;
      hAudioInSai.Init.PdmInit.Activation = ENABLE;
      hAudioInSai.Init.PdmInit.ClockEnable = SAI_PDM_CLOCK2_ENABLE;
      hAudioInSai.FrameInit.FrameLength = 16;
      if (AudioInCtx[Instance].ChannelsNbr <= 2U) {
        hAudioInSai.Init.PdmInit.MicPairsNbr = 1;
        hAudioInSai.Init.AudioFrequency = ((PDM_Clock_Freq * 1000U)
            / hAudioInSai.FrameInit.FrameLength) * 2U;
      } else {
        hAudioInSai.Init.PdmInit.MicPairsNbr = 2;
        hAudioInSai.Init.AudioFrequency = ((PDM_Clock_Freq * 1000U)
            / hAudioInSai.FrameInit.FrameLength) * 4U;
      }

      hAudioInSai.FrameInit.ActiveFrameLength = 1;
      hAudioInSai.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
      hAudioInSai.FrameInit.FSPolarity = SAI_FS_ACTIVE_HIGH;
      hAudioInSai.FrameInit.FSOffset = SAI_FS_FIRSTBIT;
      hAudioInSai.SlotInit.FirstBitOffset = 0;
      hAudioInSai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
      hAudioInSai.SlotInit.SlotNumber = 1;
      hAudioInSai.SlotInit.SlotActive = 0x00000003;

      if (HAL_SAI_Init(&hAudioInSai) != HAL_OK) {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      /* Enable SAI to generate clock used by audio driver */
      __HAL_SAI_ENABLE(&hAudioInSai);

      if (COMPANION_AUDIO_PDMToPCM_Init(
          Instance, AudioInCtx[0].SampleRate, AudioInCtx[0].ChannelsNbr,
          AudioInCtx[0].ChannelsNbr) != BSP_ERROR_NONE) {
        return BSP_ERROR_NO_INIT;
      }
    } else {
      return BSP_ERROR_WRONG_PARAM;
    }

    /* Update BSP AUDIO IN state */
    AudioInCtx[Instance].State = AUDIO_IN_STATE_STOP;
    /* Return BSP status */
  }
  return BSP_ERROR_NONE;
}

int32_t COMPANION_AUDIO_Record(uint32_t Instance, uint8_t *pBuf,
                               uint32_t NbrOfBytes) {
  if (Instance >= (AUDIO_IN_INSTANCES_NBR - 1U)) {
    return BSP_ERROR_WRONG_PARAM;
  } else {
    AudioInCtx[Instance].pBuff = (uint16_t*) pBuf;

    if (Instance == 0U) {
      UNUSED(NbrOfBytes);
      if (AudioInCtx[Instance].ChannelsNbr > 2U) {
        if (HAL_SPI_Receive_DMA(&hAudioInSPI, (uint8_t*) SPI_InternalBuffer,
                                (uint16_t) AudioInCtx[Instance].Size)
            != HAL_OK) {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }

      if (AudioInCtx[Instance].ChannelsNbr != 1U) {
        if (AUDIO_IN_Timer_Start() != HAL_OK) {
          return BSP_ERROR_PERIPH_FAILURE;
        }
      }

      if (HAL_I2S_Receive_DMA(&hAudioInI2s, I2S_InternalBuffer,
                              (uint16_t) AudioInCtx[Instance].Size / 2U)
          != HAL_OK) {
        return BSP_ERROR_PERIPH_FAILURE;
      }
      /* Update BSP AUDIO IN state */
      AudioInCtx[Instance].State = AUDIO_IN_STATE_RECORDING;
    } else {
      return BSP_ERROR_WRONG_PARAM;
    }
  }
  /* Return BSP status */
  return BSP_ERROR_NONE;
}

int32_t COMPANION_AUDIO_PDMToPCM(uint32_t Instance, uint16_t *PDMBuf,
                                 uint16_t *PCMBuf) {
  if (Instance != 0U) {
    return BSP_ERROR_WRONG_PARAM;
  } else {
    uint32_t index;

    for (index = 0; index < AudioInCtx[Instance].ChannelsNbr; index++) {
      if (AudioInCtx[Instance].SampleRate == 8000U) {
        uint16_t Decimate_Out[8U * N_MS_PER_INTERRUPT];
        uint32_t ii;
        uint16_t PDM_Filter_Out[16U * N_MS_PER_INTERRUPT];

        (void) PDM_Filter(&((uint8_t*) (PDMBuf))[index], PDM_Filter_Out,
                          &PDM_FilterHandler[index]);
        (void) arm_fir_decimate_q15(&ARM_Decimator_State[index],
                                    (q15_t*) &(PDM_Filter_Out),
                                    (q15_t*) &(Decimate_Out),
                                    DECIMATOR_BLOCK_SIZE);
        for (ii = 0; ii < (8U * N_MS_PER_INTERRUPT); ii++) {
          PCMBuf[(ii * AudioInCtx[Instance].ChannelsNbr) + index] =
              Decimate_Out[ii];
        }
      } else {
        switch (AudioInCtx[Instance].BitsPerSample) {
          case AUDIO_RESOLUTION_16b:
            (void) PDM_Filter(&((uint8_t*) (PDMBuf))[index],
                              (uint16_t*) &(PCMBuf[index]),
                              &PDM_FilterHandler[index]);
            break;
          case AUDIO_RESOLUTION_24b:
            (void) PDM_Filter(&((uint8_t*) (PDMBuf))[index],
                              &((uint8_t*) (PCMBuf))[3U * index],
                              &PDM_FilterHandler[index]);
            break;
          case AUDIO_RESOLUTION_32b:
            (void) PDM_Filter(&((uint8_t*) (PDMBuf))[index],
                              (uint32_t*) &(PCMBuf[index]),
                              &PDM_FilterHandler[index]);
            break;
          default:
            break;
        }
      }
    }
  }
  return BSP_ERROR_NONE;
}


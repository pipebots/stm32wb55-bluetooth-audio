# Microphone setup

The part used is the ST IMP34DT05 and the datasheet is [here.](https://www.st.com/resource/en/datasheet/imp34dt05.pdf)

The microphone connections on the companion board are:

| Schematic Label| STM32WB5MMG Pin |
|---|---|
| PDM_CLK | PA8 |
| PDM_DI | PA9 |
| MIC_PWR | PC10 |
| | |

The STM32CubeMX settings are show below:

![Companion board SAI1 settings](CompanionSA1settings.png "Companion board SAI1 settings")

NOTE: The companion board microphone can only work if the IMU is NOT fitted as pin `PC3 - IMU_PWR` is allocated by STM32CubeIDE for the `SA1` interface.

## Driver software

Now to find out what ST have produced to help us mere mortals write some code.

Started with this [series of videos](https://www.youtube.com/playlist?list=PLwqrqfRKpbeIxgnKq1_hTCULCaL7LBHmN).  Whilst interesting, it didn't point me to any code examples but did lead to the following application notes:

* [AN4426: Tutorial for MEMs microphones](https://www.st.com/resource/en/application_note/an4426-tutorial-for-mems-microphones-stmicroelectronics.pdf).  Useful background info on how a MEMs microphone works.
* [AN5027: Interfacing PDM digital microphones using STM32 MCUs and MPUs](https://www.st.com/resource/en/application_note/an5027-interfacing-pdm-digital-microphones-using-stm32-mcus-and-mpus-stmicroelectronics.pdf).  This doc does kind of show you how to set up the SAI block on the chip (the GUI has changed but it does explain setting up most of the values).

From the above, I figured out that there is a `PDM_Filter_handler` library.  Searching the forums led me to this example code:
<https://github.com/STMicroelectronics/x-cube-memsmic1/tree/main/Projects/STM32WB55RG-Nucleo/Demonstration/CCA02M2/Microphones_Streaming>

The software expansion pack [X-CUBE-MEMSMIC1](https://github.com/STMicroelectronics/x-cube-memsmic1) has the code I need so I installed it and then used the example code to hack something together.

We need to use DMA transfer to get the data.  I used the code I wrote for the serial comms receiver as inspiration although it worked in a different way to the microphone.

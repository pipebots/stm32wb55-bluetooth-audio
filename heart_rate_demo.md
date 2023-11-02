# Notes on setting up Bluetooth demo

## Getting something working

Working through this example.

https://www.youtube.com/watch?v=i10X4Blr8ns&t=299s


Must set up USART1 TX DMA as first one to enable the option CFG_HW_USART1_DMA_TX_SUPPORTED.  If not, you get the following red message:

```text
To enable USART1 DMA TX, CFG_HW_USART1_ENABLED shall be enabled & you need to add relevant DMA request in USART1 DMA Settings panel.
```

which doesn't tell you how to fix the problem!

This example didn't work, so back to the drawing board.

## Take 2

This set of videos looks useful.

<https://www.youtube.com/playlist?list=PLnMKNibPkDnG9JRe2fbOOpVpWY7E4WbJ->

### Video 3

<https://www.youtube.com/watch?v=YSyaH5v3hys>

This video is out of date.  STM32CubeMX is no longer needed as it has been
integrated into the STM32CubeIDE.  STM32CubeWB is no longer available.

Installed the following tools:

* STM32CubeIDE
* STM32CubeProgrammer
* STM32CubeMonitorRF

### Video 5

The first example, video 5, is a heart rate monitor and seems like the first
one to use.  However, the FUS and Wireless stack needs to be updated first, so
off to video 9.

### Video 9

<https://www.youtube.com/watch?v=1LvfBC_P6eg>

First, locate the binaries directory.  On my PC, they were here:
`~/STM32Cube/Repository/STM32Cube_FW_WB_V1.17.3/Projects/STM32WB_Copro_Wireless_Binaries/STM32WB5x`

The release notes `Release_Notes.html` for this binary are important as you
need to find the start address for the start of each image, so open them up
and find the images that you want to use.

First update the Firmware Update Service (FUS) binary.

1. Start `STM32CubeProgrammer`.
2. Click on the firmware update service icon on the left hand side.
3. Connect to the dev board using the `ST-LINK` option.
4. Press "Start FUS".  It should start.
5. Press `Read FUS Info` button. You should see some info in the boxes under the button.
6. Select the file `stm32wb5x_FUS_fw.bin`.
7. Enter the start address, `0x080EC000` for the dev board.
8. Press `Firmware Upgrade` button and wait for it to complete.

Second, update the radio firmware.  These instructions install the BLE firmware
that is used by the ST example code.

1. Press the `StartFUS` button and wait until started.
2. Select the file `stm32wb5x_BLE_Stack_full_fw.bin`.
3. Set the start address, `0x080CE000`.
4. Press `Firmware Upgrade` button and wait for it to complete.

If all goes well, disconnect and go back to video 5.

### Video 5, take 2

The code has moved on from the video but it mostly works.  This is what you
need to do:

1. Import the heart rate code into the STM32cubeIDE from here:
`~/STM32Cube/Repository/STM32Cube_FW_WB_V1.17.3/Projects/P-NUCLEO-WB55.Nucleo/Applications/BLE/BLE_HeartRate/STM32CubeIDE/.project`
2. Make the following changes:

    ```diff
    diff --git a/BLE/BLE_HeartRate/Core/Inc/app_conf.h b/BLE/BLE_HeartRate/Core/Inc/app_conf.h
    index eb1f8f5..a3f1381 100644
    --- a/BLE/BLE_HeartRate/Core/Inc/app_conf.h
    +++ b/BLE/BLE_HeartRate/Core/Inc/app_conf.h
    @@ -510,17 +510,17 @@ typedef enum
    * keep debugger enabled while in any low power mode when set to 1
    * should be set to 0 in production
    */
    -#define CFG_DEBUGGER_SUPPORTED    0
    +#define CFG_DEBUGGER_SUPPORTED    1

    /**
    * When set to 1, the traces are enabled in the BLE services
    */
    -#define CFG_DEBUG_BLE_TRACE     0
    +#define CFG_DEBUG_BLE_TRACE     1

    /**
    * Enable or Disable traces in application
    */
    -#define CFG_DEBUG_APP_TRACE     0
    +#define CFG_DEBUG_APP_TRACE     1

    #if (CFG_DEBUG_APP_TRACE != 0)
    #define APP_DBG_MSG                 PRINT_MESG_DBG
    @@ -546,8 +546,8 @@ typedef enum
    * When both are set to 0, no trace are output
    * When both are set to 1,  CFG_DEBUG_TRACE_FULL is selected
    */
    -#define CFG_DEBUG_TRACE_LIGHT     1
    -#define CFG_DEBUG_TRACE_FULL      0
    +#define CFG_DEBUG_TRACE_LIGHT     0
    +#define CFG_DEBUG_TRACE_FULL      1

    #if (( CFG_DEBUG_TRACE != 0 ) && ( CFG_DEBUG_TRACE_LIGHT == 0 ) && (CFG_DEBUG_TRACE_FULL == 0))
    #undef CFG_DEBUG_TRACE_FULL
    ```

3. Build and run the code.

When I first tried this, there was no trace output and there was no debug break point.  Finally read the `readme.txt` file and found out I needed to use the firmware file `stm32wb5x_BLE_Stack_full_fw.bin`, start address `0x080CE000`, so installed that.  Updated installation instructions to show exactly how.

Now the trace output was found on `/dev/ttyACM0` and the app worked.  The
video does not tell you how to use the Bluetooth app so here goes.

1. On an Android phone, download and install the `ST BLE Sensor` app using Google Play.  There is an equivalent app for iPhones.
2. Start the the app on the P-Nucleo board.
3. On your phone, open the `ST BLE Sensor` app.
4. Tap `Connect one device`.
5. You should see an ST app `HRSTM` in the list.  Tap that.
6. The app should  connect to the app (takes a few seconds) and you should see a big pulsing heart and some other stuff.


## Notes

For the final version, we need thread and BLE working so the firmware package
should be one of the two listed below.

| Firmware | Start address |
|---|---|
| `stm32wb5x_BLE_Thread_dynamic_fw.bin` | `0x0805D000` |
| `stm32wb5x_BLE_Thread_static_fw.bin` | `0x0805F000` |

So far, I have not found any documentation about which to use.

TODO Work out which one to use.
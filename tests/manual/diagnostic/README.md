# Sidewalk Diagnostic App

This application provides diagnostic capabilities for Sidewalk protocol testing.

## Prerequisites

Hardware:
- nRF52840 DK
- Smetech SX1262MB2xAS shield

Software:
- nRF Connect SDK with Sidewalk Add-on
- RTT Viewer

## Build and Test

1. Build the application:

    ```bash
    west build -b nrf52840dk -p
    ```

2. Flash with mass erase (recommended for first time or when experiencing issues):

    ```bash
    west flash --erase
    ```

3. Connect to the board via RTT:
   - Install JLinkRTTViewer or JLinkRTTClient from SEGGER
   - Connect the nRF52840 board via USB 
   - Launch JLinkRTTViewer and configure:
     - Target Device: NRF52840_XXAA
     - Target Interface: SWD
     - Speed: 4000 kHz
   - Click OK to connect
   - Select the RTT channel 0 to view diagnostic logs

   Note: If you don't see any output, try resetting the board while RTT Viewer is connected.

4. After successful flashing and connecting to RTT, you should see output similar to this (you may need to reset the board):

    ```log
    *** Booting My Application v2.8.99-9d8653406acf ***
    *** Using nRF Connect SDK v2.9.99-10dd3ddbf256 ***
    *** Using Zephyr OS v4.0.99-83390163009b ***
    [00:00:00.001,159] <inf> sid_template: Sidewalk example started!
    ----------------------------------------------------------------
    Sidewalk SDK        = 1.18.0.18
    APP_BUILD_VERSION   = v2.9.0-rc2-77-g9d8653406acf
    APP_NAME            = sidewalk
    build time          = Mar 27 2025 13:07:24
    board               = nrf52840dk/nrf52840
    ----------------------------------------------------------------
    --> [00:00:00.057,769] <inf> sid_thread: sid diagnostics application started...
    I: SID Diagnostics, v3.
    I: Checking persistent store for test mode configuration
    I: starting asd_rtos_cli loop
    -->
    ```

5. Available CLI commands:
   - print - Print values from device
     - fwversion - Print FW version
   - tm - RF test commands
     - phy - Get/Set physical layer radio settings
     - cw - Turn on/off continuous carrier wave
     - cpbl - Turn on/off continuous preamble wave
     - mod - Turn on/off continuous modulated tx
     - state - Get/Set Radio state
     - ping - Send PING packet for roundtrip RSSI test
     - pwr - Set test mode tx power
     - pcfg - Get/Set PA configuration
     - scan - Do a channel scan and dump noise floor values
     - fhop - Turn on/off frequency hopping test
     - reset_rx_counters - Reset RX counters
     - print_rx_counters - Print RX counters
     - last_packet - Print status of last tx/rx packet
     - last_rssi - Print RSSI of last rx packets
   - gpio - GPIO test commands
     - pin_test - Set/Get gpio pin test


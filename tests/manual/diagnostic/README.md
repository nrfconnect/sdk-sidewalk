# Sidewalk Diagnostic App

This application provides diagnostic capabilities for Sidewalk protocol testing.

## Prerequisites

Hardware:
- nRF52840 DK: https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK
- Smetech sx126x mbed shield: https://www.semtech.com/products/wireless-rf/lora-connect/sx1262mb2cas

Software:
- nRF Connect SDK with Sidewalk Add-on: https://docs.nordicsemi.com/bundle/sidewalk_latest/page/setting_up_sidewalk_environment/setting_up_sdk.html
- RTT Viewer: https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/

Configuration:

The pin configuration for SPI and Semtech SX1262 radio can be modified in `boards/nrf52840dk_nrf52840.overlay`. The default pin mapping is:

SPI pins:
- SCK: P1.15 (0xF)
- MISO: P1.14 (0xE) 
- MOSI: P1.13 (0xD)
- CS: P1.08 (0x8)

Semtech SX1262 pins:
- CS: P1.08 (0x8)
- Reset: P0.03 (0x3)
- Busy: P1.04 (0x4)
- Antenna Enable: P1.10 (0xA)
- DIO1: P1.06 (0x6)

To use different pins, modify the corresponding GPIO pin numbers in the overlay file:

```dts
	nrfx_spi_gpios{
		compatible = "gpio-keys";
		nrfx_spi_sck: sck {
			gpios = <&gpio1 0xF GPIO_ACTIVE_LOW>;
			label = "spi_sck";
		};
		nrfx_spi_miso: miso {
			gpios = <&gpio1 0xE GPIO_ACTIVE_HIGH>;
			label = "spi_miso";
		};
		nrfx_spi_mosi: mosi {
			gpios = <&gpio1 0xD GPIO_ACTIVE_HIGH>;
			label = "spi_mosi";
		};
		nrfx_spi_cs: cs {
			gpios = <&gpio1 0x8 GPIO_ACTIVE_HIGH>;
			label = "spi_cs";
		};
	};

	semtech_sx1262_gpios{
		compatible = "gpio-keys";
		semtech_sx1262_cs: cs {
			gpios = <&gpio1 0x8 GPIO_ACTIVE_LOW>;
			label = "semtech_sx1262 CS";
		};
		semtech_sx1262_reset_gpios: reset {
			gpios = <&gpio0 0x3 GPIO_ACTIVE_LOW>;
			label = "semtech_sx1262 Reset";
		};
		semtech_sx1262_busy_gpios: busy {
			gpios = <&gpio1 0x4 GPIO_ACTIVE_HIGH>;
			label = "semtech_sx1262 Busy";
		};
		semtech_sx1262_antenna_enable_gpios: antena_enable {
			gpios = <&gpio1 0xa GPIO_ACTIVE_HIGH>;
			label = "semtech_sx1262 Antena Enable";
		};
		semtech_sx1262_dio1_gpios: dio1 {
			gpios = <&gpio1 0x6 GPIO_ACTIVE_HIGH>;
			label = "semtech_sx1262 DIO1";
		};
	};
```

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


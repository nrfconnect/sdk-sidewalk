.. _sidewalk_location:

Sidewalk location
#################

.. contents::
   :local:
   :depth: 2

The Sidewalk location sample demonstrates how to report the position of an end device over Amazon Sidewalk with the Sidewalk Location Library.
The device starts a location run at a fixed interval, beginning with Bluetooth® LE gateway location and, on devices with the LR1110 radio, falling back to Wi-Fi and GNSS location.
A button press sends a message for which the cloud resolves the device position.

Requirements
************

The sample supports the following development kits:

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - Board name
     - Build target
     - Shield
   * - nRF54L15 TAG
     - :doc:`nrf54l15tag <zephyr:boards/nordic/nrf54l15tag/doc/index>`
     - ``nrf54l15tag/nrf54l15/cpuapp``
     - --
   * - `nRF54LM20 DK <nRF54LM20A Product Page_>`_
     - :doc:`nrf54lm20dk <zephyr:boards/nordic/nrf54lm20dk/doc/index>`
     - | ``nrf54lm20dk/nrf54lm20a/cpuapp``
       | ``nrf54lm20dk/nrf54lm20b/cpuapp``
     - :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` (``nrf_sidewalk_eb``)

The location methods and Sidewalk links depend on the hardware:

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - Location levels
     - Sidewalk links
   * - nRF54L15 TAG
     - Bluetooth LE (L1)
     - Bluetooth LE
   * - nRF54LM20 DK with the nRF Sidewalk EB
     - Bluetooth LE (L1), Wi-Fi (L3), GNSS (L4)
     - Bluetooth LE, LoRa

For details on the location levels, see :ref:`location_services` and the `Amazon Sidewalk Location Library Developer Guide`_.

You also need the following:

* A Sidewalk gateway, for example an `Echo 4th Gen`_.
* A Sidewalk device created in AWS IoT Core for Amazon Sidewalk with positioning enabled, see :ref:`setting_up_sidewalk_prototype`.
* For the nRF54L15 TAG, an nRF54L DK to program the TAG through the **DEBUG OUT** connector, and a CR2032 coin cell or an external 3.3 V supply.
  See :doc:`nrf54l15tag <zephyr:boards/nordic/nrf54l15tag/doc/index>`.

Overview
********

The application runs a :doc:`Zephyr State Machine Framework <zephyr:services/smf/index>` state machine in the main thread, which also calls ``sid_process()``.
Between events, the device is in System ON sleep.

.. code-block:: none

   boot ──► Connecting ──► Locating ◄──► Idle

The sample uses the following states:

Connecting
   The application starts Sidewalk with the Bluetooth LE auto connect policy and waits until the device is registered and time synced.
   Then, it initializes the Location Library and starts the first location run.

Locating
   A location run is in progress.
   When the run ends, the application goes to the Idle state.

Idle
   The application waits for the next location run or a button press.

Location runs
=============

Every :ref:`CONFIG_SID_LOCATION_INTERVAL_S <CONFIG_SID_LOCATION_INTERVAL_S>` seconds, the application calls ``sid_location_run()`` with the levels from the ``levels`` array in :file:`src/main.c`:

* nRF54L15 TAG -- L1.
* nRF54LM20 DK with the nRF Sidewalk EB -- L1, L3, and L4.

The Location Library tries the levels in order and stops at the first level that sends a result.
It moves to the next level when a level is not available, or when the Wi-Fi or GNSS scan finds nothing.
When an uplink fails, the library ends the run, and the application starts a new run with the remaining levels.
The Wi-Fi and GNSS results are sent over Bluetooth LE or LoRa.

An L1 run does not trigger the Bluetooth LE auto connect.
If the Bluetooth LE link is down, the application requests a connection with ``sid_ble_bcn_connection_request()`` and starts the run when the link is up.

The first run after boot can start with Wi-Fi, because L1 is not available until the gateway reports location support.
A Wi-Fi run succeeds when the scan result is sent, even if the cloud cannot resolve the position, so the library does not try GNSS.

Location with a message
=======================

A button press sends ``Message <counter>`` over Bluetooth LE with ``sid_put_msg()``.
The message descriptor sets ``resolve_location`` to ``SID_TX_LOCATION_LINK_RESOLVE``, so the cloud resolves the device position from the gateway that receives the message.
The Location Library reports ``SID_LOCATION_RESOLVE_MSG_SENT`` or ``SID_LOCATION_RESOLVE_MSG_TIMEOUT`` in the ``on_update`` callback.

Power consumption
=================

The release configuration disables logs, the console, and the UART.

On the nRF54LM20 DK, LoRa uses the asynchronous profile, and the LR1110 sleeps between scans.
The level shifters of the nRF Sidewalk EB add several hundred µA, see :ref:`nrf_sidewalk_eb`.

Between runs, Bluetooth LE advertising is the main load.
To use a slower advertising interval, set the ``CONFIG_SIDEWALK_BLE_ADV_INT_SLOW`` Kconfig option.

User interface
**************

After each location run or message, one LED blinks to show the result.
The button is ignored during a location run.

nRF54L15 TAG
============

Button (P0.00):
   Sends a message with location resolution.

Blue LED:
   Blinks when the position is sent with Bluetooth LE, or when the message is sent.

Red LED:
   Blinks when a location run or a message fails.
   One blink at boot means that the device has no manufacturing data.

nRF54LM20 DK
============

**Button 0**:
   Sends a message with location resolution.

**LED 0**:
   Blinks when the position is sent with Bluetooth LE, or when the message is sent.

**LED 1**:
   Blinks when the position is sent with Wi-Fi.

**LED 2**:
   Blinks when the position is sent with GNSS.

**LED 3**:
   Blinks when a location run or a message fails.
   One blink at boot means that the device has no manufacturing data.

Configuration
*************

See :doc:`Configuring and building <nrf:app_dev/config_and_build/index>` for information about how to permanently or temporarily change the configuration.

Configuration options
=====================

Check and configure the following Kconfig options:

.. _CONFIG_SID_LOCATION_INTERVAL_S:

CONFIG_SID_LOCATION_INTERVAL_S - Location run interval
   Sets the time between location runs, in seconds.
   The default value is ``60``.

.. _CONFIG_SID_LOCATION_WIFI:

CONFIG_SID_LOCATION_WIFI - Wi-Fi location
   Enables Wi-Fi location (L3) with the LR1110.
   Enabled by default when the LR1110 is used.

.. _CONFIG_SID_LOCATION_GNSS:

CONFIG_SID_LOCATION_GNSS - GNSS location
   Enables GNSS location (L4) with the LR1110.
   Enabled by default when the LR1110 is used.

Configuration files
===================

The sample provides the following configuration files in the sample directory:

.. list-table::
   :header-rows: 1

   * - Build type
     - File name
     - Description
   * - Debug (default)
     - :file:`prj.conf`
     - Debug version of the application, with logs.
   * - Release
     - :file:`prj_release.conf`
     - Release version of the application, without logs, console, serial, and asserts.

The :file:`boards` directory contains the LED aliases (``loc-led-ble``, ``loc-led-wifi``, ``loc-led-gnss``, ``loc-led-error``) and the flash partitions.
The sample has no bootloader, so the application starts at address ``0x0``.
The ``storage`` and ``mfg_storage`` partitions use the same addresses as in the :ref:`sidewalk_end_device` sample.

Building and running
********************

This sample can be found under :file:`samples/sid_location`.

For general instructions on building the sample, follow the steps in the :doc:`Building and programming an application <nrf:app_dev/programming>` documentation.

Build the sample in the sample directory with the following commands:

.. tabs::

   .. tab:: nrf54l15tag/nrf54l15/cpuapp

      .. code-block:: console

         west build -b nrf54l15tag/nrf54l15/cpuapp

   .. tab:: nrf54lm20dk/nrf54lm20a/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20a/cpuapp --shield nrf_sidewalk_eb

   .. tab:: nrf54lm20dk/nrf54lm20b/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20b/cpuapp --shield nrf_sidewalk_eb

For the release build, add ``-- -DFILE_SUFFIX=release`` to the command.

The nRF54L15 TAG has no UART connection.
To get the logs of the debug build over RTT, add the ``rtt-console`` snippet with ``-S rtt-console``.
The snippet needs the SEGGER module.
If your workspace uses the standalone Sidewalk manifest, add ``segger`` to the ``name-allowlist`` in the :file:`west.yml` file, or pass the module path with ``-DEXTRA_ZEPHYR_MODULES``.

Programming and provisioning
============================

Program the application and write the device credentials to the ``mfg_storage`` partition:

.. code-block:: console

   west flash
   west sid provision -C certificate.json

For details on provisioning, see :ref:`setting_up_sidewalk_prototype`.

To program the nRF54L15 TAG, connect it to the **DEBUG OUT** connector of an nRF54L DK and power it.
If the TAG is not powered, the DK programs its own SoC.
Before programming, check that ``nrfutil device device-info`` reports ``nRF54L15`` as the device name.

.. caution::
   The ``west flash --erase`` command and a full chip erase delete the manufacturing data and the Sidewalk registration.

Testing
=======

See :doc:`Testing and debugging an application <nrf:test_and_optimize>` for information on testing and debugging in the nRF Connect SDK.

After programming and provisioning the device, complete the following steps to test it:

1. Place the device in range of the Sidewalk gateway.
#. For the debug build, connect to the device logs:

   * nRF54LM20 DK -- Connect to the first virtual serial port of the DK with a terminal emulator (115200 baud, 8 data bits, 1 stop bit, no parity).
   * nRF54L15 TAG -- Connect to the RTT console, see :doc:`Logging in nRF Connect SDK <nrf:test_and_optimize/logging>`.

#. Wait until the device is registered and time synced, and the first location run ends.
   An LED blinks to show the result.
#. Press the button to send a message with location resolution.
   An LED blinks when the message is sent.

Sample output
=============

The debug build prints messages similar to the following:

.. code-block:: console

   <inf> sid_location_app: Location run waits for a Bluetooth LE connection
   <inf> sid_ble_conn: BT Connected
   <inf> sid_location_app: Sidewalk state 0, registered 1, time synced 1, links 0x1
   <inf> sid_location_app: Location run started at L1
   <inf> sid_location_app: Location status 4, err 0, effort 1, link 0x1
   <inf> sid_location_app: "Message 1" with location resolution queued

Troubleshooting
===============

The red LED or **LED 3** blinks once at boot
   The device has no manufacturing data.
   Run ``west sid provision``.

No location run starts after programming
   The device is not registered or time synced.
   Check that the gateway is in range and has Sidewalk enabled.

Every location run fails
   Bluetooth LE location needs a gateway in range.
   Wi-Fi location needs visible access points, and GNSS location needs open sky.

Dependencies
************

This sample uses the following Sidewalk libraries:

* Sidewalk API (:file:`sid_api.h`)
* Sidewalk Location Library (:file:`sid_location.h`)

It uses the following nRF Connect SDK library:

* :doc:`DK Buttons and LEDs <nrf:libraries/others/dk_buttons_and_leds>`

It uses the following Zephyr subsystem:

* :doc:`State Machine Framework <zephyr:services/smf/index>`

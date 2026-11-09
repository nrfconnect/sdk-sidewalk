.. _sidewalk_location:

Sidewalk location
#################

.. contents::
   :local:
   :depth: 2

This sample sends the device position to AWS over Amazon Sidewalk with the Sidewalk location library.
It runs a location run periodically, starting with Bluetooth® LE and, on the nRF54LM20 DK, falling back to Wi-Fi and GNSS.
A button press sends a message for which the cloud resolves the position.

Requirements
************

The sample supports the following development kits:

.. list-table::
   :header-rows: 1

   * - Hardware platform
     - Build target
     - Location methods
     - Sidewalk links
   * - :doc:`nRF54L15 TAG <zephyr:boards/nordic/nrf54l15tag/doc/index>`
     - ``nrf54l15tag/nrf54l15/cpuapp``
     - Bluetooth LE (L1)
     - Bluetooth LE
   * - :doc:`nRF54LM20 DK <zephyr:boards/nordic/nrf54lm20dk/doc/index>` with the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`
     - | ``nrf54lm20dk/nrf54lm20a/cpuapp``
       | ``nrf54lm20dk/nrf54lm20b/cpuapp``
     - Bluetooth LE (L1), Wi-Fi (L3), GNSS (L4)
     - Bluetooth LE, LoRa

You also need the following:

* A Sidewalk gateway, for example an `Echo 4th Gen`_.
* An AWS account with AWS IoT Core for Amazon Sidewalk, see :ref:`setting_up_sidewalk_prototype`.
* For the nRF54L15 TAG, a development kit with a **DEBUG OUT** connector, for example the :doc:`nRF54L15 DK <zephyr:boards/nordic/nrf54l15dk/doc/index>`.

The sample does not use LoRa location (L2) or FSK.
For details on the location levels, see :ref:`location_services` and the `Amazon Sidewalk Location Library Developer Guide`_.

Overview
********

The application runs a :doc:`Zephyr State Machine Framework <zephyr:services/smf/index>` state machine in the main thread.
Between events, the device is in System ON sleep.

.. code-block:: none

   boot ──► Connecting ──► Locating ◄──► Idle

Boot
   The application initializes the Sidewalk platform, checks the manufacturing data, and starts Sidewalk with the Bluetooth LE auto connect policy.

Connecting
   The application waits until Sidewalk is registered and time synced.
   Then, it initializes the location library, as described in :ref:`known_issues`, and starts the first location run.

Locating
   A location run is in progress.
   When the run ends, the application goes to **Idle**.

Idle
   The application waits for the periodic timer or a button press.

Location runs
=============

Every ``CONFIG_SID_LOCATION_INTERVAL_S`` seconds, the application passes the ``levels`` array from :file:`src/main.c` to ``sid_location_run()``:

* nRF54L15 TAG -- L1.
* nRF54LM20 DK with the nRF Sidewalk EB -- L1, L3, and L4.
  Disable L3 or L4 with ``CONFIG_SID_LOCATION_WIFI`` or ``CONFIG_SID_LOCATION_GNSS``.

The library moves to the next level when a level is not available or when the Wi-Fi or GNSS scan finds nothing.
After a failed uplink, the library stops, and the application starts a new run with the remaining levels.
Wi-Fi and GNSS results are sent over Bluetooth LE or LoRa.

An L1 run does not trigger the Bluetooth LE auto connect.
If the Bluetooth LE link is down, the application requests a connection with ``sid_ble_bcn_connection_request()`` and starts the run when the gateway connects.

L1 is not available until the gateway reports location support, so the first run after boot can start with Wi-Fi.
A Wi-Fi run succeeds when the scan is sent, even if the cloud cannot resolve the position, so GNSS is not tried.

Location with a message
=======================

The button sends ``Message <counter>`` with ``sid_put_msg()``, with ``resolve_location`` set to ``SID_TX_LOCATION_LINK_RESOLVE`` in the message descriptor.
The message is sent over Bluetooth LE, and the cloud resolves the position from the receiving gateway.
The library reports ``SID_LOCATION_RESOLVE_MSG_SENT`` or ``SID_LOCATION_RESOLVE_MSG_TIMEOUT`` in the ``on_update`` callback.

User interface
==============

.. list-table::
   :header-rows: 1

   * - Action
     - nRF54L15 TAG
     - nRF54LM20 DK
   * - Send a message with location
     - Button (P0.00)
     - **Button 0**

The button is ignored during a location run.

After each run or message, one LED blinks:

.. list-table::
   :header-rows: 1

   * - Result
     - nRF54L15 TAG
     - nRF54LM20 DK
   * - Position sent with Bluetooth LE, or message sent
     - Blue
     - **LED 0**
   * - Position sent with Wi-Fi
     - --
     - **LED 1**
   * - Position sent with GNSS
     - --
     - **LED 2**
   * - Failure, or no manufacturing data at boot
     - Red
     - **LED 3**

Configuration
*************

.. list-table::
   :header-rows: 1

   * - Kconfig option
     - Default
     - Description
   * - ``CONFIG_SID_LOCATION_INTERVAL_S``
     - 60
     - Interval between location runs, in seconds.
   * - ``CONFIG_SID_LOCATION_WIFI``
     - ``y`` with the LR1110
     - Wi-Fi location (L3).
   * - ``CONFIG_SID_LOCATION_GNSS``
     - ``y`` with the LR1110
     - GNSS location (L4).

Configuration files:

* :file:`prj.conf` -- Debug configuration.
* :file:`prj_release.conf` -- Release configuration, without logs, console, serial, and asserts.
* :file:`boards/*.overlay` -- LED aliases (``loc-led-ble``, ``loc-led-wifi``, ``loc-led-gnss``, ``loc-led-error``) and flash partitions without a bootloader.

Building
********

Run the commands from the west workspace directory.

nRF54L15 TAG, release:

.. code-block:: console

   west build -p -b nrf54l15tag/nrf54l15/cpuapp sidewalk/samples/sid_location -d build_tag -- -DFILE_SUFFIX=release

nRF54L15 TAG, debug with logs over RTT:

.. code-block:: console

   west build -p -b nrf54l15tag/nrf54l15/cpuapp sidewalk/samples/sid_location -d build_tag -S rtt-console

The ``rtt-console`` snippet needs the SEGGER module.
If your workspace uses only the Sidewalk manifest, add ``segger`` to the ``name-allowlist`` in :file:`west.yml`, or pass the module path with ``-DEXTRA_ZEPHYR_MODULES``.

nRF54LM20 DK with the nRF Sidewalk EB, release:

.. code-block:: console

   west build -p -b nrf54lm20dk/nrf54lm20a/cpuapp sidewalk/samples/sid_location -d build_lm20 -- -DSHIELD=nrf_sidewalk_eb -DFILE_SUFFIX=release

For the debug build, remove ``-DFILE_SUFFIX=release``.
The logs are on the first UART of the DK.

AWS setup
*********

Once per AWS account:

1. Create the destination and its role, see :file:`tools/location_demo/README.md`.
2. Create a Sidewalk device profile.

For each device, in the AWS IoT console:

1. Go to :guilabel:`Manage → LPWAN devices → Devices` and add a Sidewalk device with the device profile and the destination.
2. Set :guilabel:`Positioning` to :guilabel:`Enabled` and select the same destination.
3. Download the device JSON file (:file:`certificate.json`).

.. caution::
   The device JSON file contains the device keys.
   Do not commit it to a repository.

Alternatively, use the `AWS CLI`_:

.. code-block:: console

   aws iotwireless create-wireless-device --type Sidewalk --name TAG-07 \
       --destination-name SidewalkLocationDemo --positioning Enabled \
       --sidewalk '{"DeviceProfileId": "<profile id>", "Positioning": {"DestinationName": "SidewalkLocationDemo"}}'
   aws iotwireless get-wireless-device --identifier-type WirelessDeviceId \
       --identifier <device id> > TAG-07_wireless_device.json
   aws iotwireless get-device-profile --id <profile id> > device_profile.json

In the provisioning step, pass these files with ``-W`` and ``-D`` instead of ``-C``.

Provisioning and programming
****************************

Program the application and write the device credentials to the ``mfg_storage`` partition:

.. code-block:: console

   west flash -d build_tag
   west sid provision -d build_tag -C certificate.json

For details, see :ref:`setting_up_sidewalk_prototype`.
Program the nRF54L15 TAG through the **DEBUG OUT** connector of a development kit.
The TAG must be powered, otherwise the development kit programs its own SoC.
Before programming, check that ``nrfutil device device-info`` reports ``nRF54L15``.

.. caution::
   ``west flash --erase`` and a full chip erase delete the manufacturing data and the Sidewalk registration.

The first location run starts after the device registers and gets time sync.

Viewing the positions
*********************

Use the viewer in :file:`tools/location_demo`, see :file:`tools/location_demo/README.md`.
It shows the positions and messages on a map at ``http://127.0.0.1:8000``.

Current consumption
*******************

Between runs, the device is in System ON sleep.
System OFF is not used, because each wake-up would need Sidewalk initialization and time sync.

* The release configuration has no logs, console, or UART.
* On the nRF54L15 TAG, the sensor, bus, and ADC drivers are not built.
* On the nRF54LM20 DK, LoRa uses the asynchronous profile, and the LR1110 sleeps between scans.
  The level shifters of the nRF Sidewalk EB add several hundred µA, see :ref:`nrf_sidewalk_eb`.
* Bluetooth LE advertising is the main load between runs.
  For a slower advertising interval, set ``CONFIG_SIDEWALK_BLE_ADV_INT_SLOW``.
  ``SID_OPTION_BLE_USER_CONFIG`` cannot change it at runtime, see :ref:`known_issues`.

Troubleshooting
***************

The red LED blinks once at boot
   The device has no manufacturing data.
   Run ``west sid provision``.

No location run after programming
   The device is not registered or time synced.
   Check that the gateway is in range and has Sidewalk enabled.

Every run fails
   Bluetooth LE location needs a gateway in range.
   Wi-Fi needs visible access points, and GNSS needs open sky.
   Check that positioning is enabled for the device in AWS.

The viewer shows no new positions
   Subscribe to ``sidewalk/location-demo/#`` in the `AWS IoT MQTT client`_.
   Check that the device uses the destination for positioning.
   With the read-only demo policy, check that the device has the demo tag.

Limitations
***********

* No firmware update over the air.
* The LED shows the method reported by the library, not the position accuracy.

.. toctree::
   :maxdepth: 1
   :caption: Subpages:

   sid_location_demo_card.rst

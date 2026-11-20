.. _sidewalk_location:

Sidewalk location
#################

.. contents::
   :local:
   :depth: 2

The sample demonstrates how to report the position of an end device with the Sidewalk Location Library.
It supports Bluetooth® LE, Wi-Fi, and GNSS location.
It also sends messages with location resolution, and supports the Device Firmware Update (DFU) over Bluetooth LE.

Requirements
************

The sample supports the following development kits:

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - Board name
     - Build target
     - Shield
   * - `nRF52840 DK <nrf52840 Product Page_>`_
     - :doc:`nrf52840dk <zephyr:boards/nordic/nrf52840dk/doc/index>`
     - ``nrf52840dk/nrf52840``
     - | --
       | LR1110MB1LCKS (``simple_arduino_adapter`` and ``semtech_lr1110mb1xxs``)
   * - nRF54L15 TAG
     - :doc:`nrf54l15tag <zephyr:boards/nordic/nrf54l15tag/doc/index>`
     - ``nrf54l15tag/nrf54l15/cpuapp``
     - --
   * - `nRF54L15 DK <nRF54L15 Product Page_>`_
     - :doc:`nrf54l15dk <zephyr:boards/nordic/nrf54l15dk/doc/index>`
     - ``nrf54l15dk/nrf54l15/cpuapp``
     - | --
       | :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` (``nrf_sidewalk_eb``)
   * - `nRF54LM20 DK <nRF54LM20A Product Page_>`_
     - :doc:`nrf54lm20dk <zephyr:boards/nordic/nrf54lm20dk/doc/index>`
     - | ``nrf54lm20dk/nrf54lm20a/cpuapp``
       | ``nrf54lm20dk/nrf54lm20b/cpuapp``
     - | --
       | :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` (``nrf_sidewalk_eb``)

The location levels and Sidewalk links depend on the hardware:

.. list-table::
   :header-rows: 1

   * - Hardware
     - Location levels
     - Sidewalk links
   * - nRF SoC only
     - Bluetooth LE (L1)
     - Bluetooth LE
   * - nRF SoC with the LR1110 (nRF Sidewalk EB or LR1110MB1LCKS)
     - Bluetooth LE (L1), Wi-Fi (L3), GNSS (L4)
     - Bluetooth LE, LoRa

For details on the location levels, see :ref:`location_services`.

Before you start, set up the Sidewalk environment, see :ref:`setting_up_the_environment`.
The device in AWS IoT Core for Amazon Sidewalk must have positioning enabled, see :ref:`setting_up_sidewalk_prototype`.

Overview
********

The application runs a :doc:`Zephyr State Machine Framework <zephyr:services/smf/index>` state machine in the Sidewalk thread.
It waits until the device is registered and time synced, then starts location runs.

Location runs
=============

The automatic runs use the managed effort of the Location Library, which selects the level of each run.
The next automatic run starts :ref:`CONFIG_SID_LOCATION_INTERVAL_S <CONFIG_SID_LOCATION_INTERVAL_S>` seconds after the last run or the last message.
The first run starts when the gateway reports location support, or one interval after the time sync.

An L1 run needs the Bluetooth LE link.
If the link is down, the application requests a connection and starts the run when the link is up, or after 30 seconds.

The Wi-Fi and GNSS buttons start a run with only that level, see `User interface`_.

Location with a message
=======================

The message button sends ``Message <counter>`` over Bluetooth LE with ``resolve_location`` set to ``SID_TX_LOCATION_LINK_RESOLVE``.
The cloud resolves the device position from the gateway that receives the message.

Device Firmware Update
======================

A long press (2 seconds) of the DFU button stops Sidewalk and starts the :doc:`Zephyr SMP server <zephyr:services/device_mgmt/ota>` over Bluetooth LE, advertising as ``Nordic``.
The DFU state ends with another long press, or when no upload starts within 5 minutes.
Then, Sidewalk starts again.

User interface
**************

After each location run or message, one LED blinks to show the result.
The error LED blinks once at boot when the device has no manufacturing data.
During a location run, only the message button works.
In the DFU state, the LEDs flash, and change the pattern when the upload starts.

nRF54L15 TAG
============

Button:
   Sends a message with location resolution.
   A long press starts or stops the DFU state.

Blue LED:
   Blinks when the position or the message is sent with Bluetooth LE.

Red LED:
   Blinks when a location run or a message fails.

nRF52840 DK, nRF54L15 DK, and nRF54LM20 DK
==========================================

**Button 0**:
   Starts an automatic location run.
   A long press starts or stops the DFU state.

**Button 1**:
   Sends a message with location resolution.

**Button 2**:
   Starts a Wi-Fi location run (L3), with the LR1110 only.

**Button 3**:
   Starts a GNSS location run (L4), with the LR1110 only.

**LED 0**:
   Blinks when the position or the message is sent with Bluetooth LE.

**LED 1**:
   Blinks when the position is sent with Wi-Fi.

**LED 2**:
   Blinks when the position is sent with GNSS.

**LED 3**:
   Blinks when a location run or a message fails.

nRF54L15 DK with the nRF Sidewalk EB
====================================

The shield uses the pins of **Button 0**, **Button 1**, **Button 2**, **LED 1**, and **LED 3**, see :ref:`nrf_sidewalk_eb`.

**Button 3**:
   Sends a message with location resolution.
   A long press starts or stops the DFU state.

**LED 0**:
   Blinks when the position or the message is sent with Bluetooth LE.

**LED 2**:
   Blinks when the position is sent with Wi-Fi or GNSS.

LED of the nRF Sidewalk EB:
   Blinks when a location run or a message fails.

Configuration
*************

See :doc:`Configuring and building <nrf:app_dev/config_and_build/index>` for information about how to permanently or temporarily change the configuration.

Configuration options
=====================

Check and configure the following Kconfig option:

.. _CONFIG_SID_LOCATION_INTERVAL_S:

CONFIG_SID_LOCATION_INTERVAL_S - Location run interval
   Sets the interval of the automatic location runs, in seconds.
   The default value is ``60``.

Configuration files
===================

The sample provides the following configuration files in the sample directory:

* :file:`prj.conf` -- Debug build, with logs.
* :file:`prj_release.conf` -- Release build, without logs.
* :file:`boards` -- LED aliases and Sidewalk flash partitions.
  On the nRF52840 DK, the secondary slot is in the external flash.
* :file:`shields` -- LED aliases for the nRF54L15 DK with the nRF Sidewalk EB.
* :file:`Kconfig.sysbuild` and :file:`sysbuild` -- MCUboot in the overwrite-only mode.
  On the nRF54L Series devices, the verification key is in the Key Management Unit (KMU).

Building and running
********************

This sample can be found under :file:`samples/sid_location`.

For general instructions on building the sample, follow the steps in the :doc:`Building and programming an application <nrf:app_dev/programming>` documentation.

Build the sample in the sample directory with the following commands:

.. tabs::

   .. tab:: nrf52840dk/nrf52840

      .. code-block:: console

         west build -b nrf52840dk/nrf52840
         west build -b nrf52840dk/nrf52840 --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

   .. tab:: nrf54l15tag/nrf54l15/cpuapp

      .. code-block:: console

         west build -b nrf54l15tag/nrf54l15/cpuapp

   .. tab:: nrf54l15dk/nrf54l15/cpuapp

      .. code-block:: console

         west build -b nrf54l15dk/nrf54l15/cpuapp
         west build -b nrf54l15dk/nrf54l15/cpuapp --shield nrf_sidewalk_eb

   .. tab:: nrf54lm20dk/nrf54lm20a/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20a/cpuapp
         west build -b nrf54lm20dk/nrf54lm20a/cpuapp --shield nrf_sidewalk_eb

   .. tab:: nrf54lm20dk/nrf54lm20b/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20b/cpuapp
         west build -b nrf54lm20dk/nrf54lm20b/cpuapp --shield nrf_sidewalk_eb

For the release build, add ``-- -DFILE_SUFFIX=release`` to the command.
The build creates the :file:`dfu_application.zip` file for the DFU in the build directory.

Programming and provisioning
============================

Program MCUboot and the application, and write the device credentials:

.. code-block:: console

   west flash --erase
   west sid provision -C certificate.json

On the nRF54L Series devices, the first programming needs ``--erase``, because ``west flash`` writes the MCUboot verification key to the KMU only with this option.
The erase also deletes the manufacturing data, so provision the device after it.

To program the nRF54L15 TAG, see :doc:`nrf54l15tag <zephyr:boards/nordic/nrf54l15tag/doc/index>`.

Testing
=======

After programming and provisioning the device, complete the following steps to test it:

1. Place the device in range of the Sidewalk gateway.
#. For the debug build, connect to the device logs, see :doc:`Logging in nRF Connect SDK <nrf:test_and_optimize/logging>`.
   The nRF54L15 TAG sends the logs over RTT, and the DKs over the first virtual serial port.
#. Wait until the first location run ends.
   An LED blinks to show the result.
#. Press the message button.
   The Bluetooth LE LED blinks when the message is sent.

To test the DFU, enter the DFU state with a long press of the DFU button, and upload the :file:`dfu_application.zip` file to the ``Nordic`` device, following the Bluetooth testing steps from the `DevZone DFU guide`_.

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

Dependencies
************

This sample uses the following Sidewalk libraries:

* Sidewalk API (:file:`sid_api.h`)
* Sidewalk Location Library (:file:`sid_location.h`)
* Sidewalk DFU library (:file:`sidewalk_dfu/nordic_dfu.h`)

It uses the following nRF Connect SDK library:

* :doc:`DK Buttons and LEDs <nrf:libraries/others/dk_buttons_and_leds>`

It uses the following Zephyr subsystems:

* :doc:`State Machine Framework <zephyr:services/smf/index>`
* :doc:`Device Management <zephyr:services/device_mgmt/ota>`

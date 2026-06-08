.. _lr11xx_firmware_update:

LR11xx firmware update
######################

.. contents::
   :local:
   :depth: 2

This sample updates the Semtech LR1110 transceiver firmware to version ``0x0401``.
The minimum LR1110 firmware version required by Sidewalk is ``0x0401``.

Use this sample when the LR1110 module runs an older firmware version and Sidewalk sub-GHz samples fail during radio initialization.
After a successful update, flash a Sidewalk application such as :ref:`sidewalk_end_device`.

Requirements
************

The sample supports the following development kits:

.. table-from-sample-yaml::

You also need one of the following LR1110 radio boards:

* `Semtech LR1110MB1LCKS`_ Arduino shield with the ``simple_arduino_adapter`` shield on the nRF54L15 DK
* :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` expansion board on the nRF54L15 DK or nRF54LM20 DK

For hardware setup and pin assignment, refer to :ref:`setting_up_hardware`.

Overview
********

The sample initializes the Sidewalk radio platform, resets the LR1110 into bootloader mode, erases flash, writes the embedded transceiver firmware image, and reboots the chip.
It then verifies that the transceiver reports the expected firmware version.

The sample does not start the Sidewalk stack or connect to a gateway.
It is intended to be run once before flashing a full Sidewalk application.

The LR1110 firmware images are available in the `LR11xx firmware images repository`_.
The firmware image with the version ``0x0401`` is already a part of the sample.

Building and running
********************

Build and flash the sample for your board and shield combination.

Semtech LR1110 Arduino shield
=============================

.. tabs::

   .. tab:: nrf52840dk/nrf52840

      .. zephyr-app-commands::
         :app: sidewalk/samples/lr11xx_firmware_update
         :board: nrf52840dk/nrf52840
         :shield: simple_arduino_adapter;semtech_lr1110mb1xxs
         :goals: build flash
         :compact:

   .. tab:: nrf54l15dk/nrf54l15/cpuapp

      .. zephyr-app-commands::
         :app: sidewalk/samples/lr11xx_firmware_update
         :board: nrf54l15dk/nrf54l15/cpuapp
         :shield: simple_arduino_adapter;semtech_lr1110mb1xxs
         :goals: build flash
         :compact:

nRF Sidewalk EB shield
======================

.. tabs::

   .. tab:: nrf54l15dk/nrf54l15/cpuapp

      .. zephyr-app-commands::
         :app: sidewalk/samples/lr11xx_firmware_update
         :board: nrf54l15dk/nrf54l15/cpuapp
         :shield: nrf_sidewalk_eb
         :goals: build flash
         :compact:

   .. tab:: nrf54lm20dk/nrf54lm20a/cpuapp

      .. zephyr-app-commands::
         :app: sidewalk/samples/lr11xx_firmware_update
         :board: nrf54lm20dk/nrf54lm20a/cpuapp
         :shield: nrf_sidewalk_eb
         :goals: build flash
         :compact:

   .. tab:: nrf54lm20dk/nrf54lm20b/cpuapp

      .. zephyr-app-commands::
         :app: sidewalk/samples/lr11xx_firmware_update
         :board: nrf54lm20dk/nrf54lm20b/cpuapp
         :shield: nrf_sidewalk_eb
         :goals: build flash
         :compact:

Testing
*******

#. Connect the development kit to your PC using a USB cable.
#. Connect the LR1110 radio shield to the development kit.
#. Connect to the kit with a terminal emulator (115200 baud, 8 data bits, 1 stop bit, no parity).
#. Build and flash the sample.
#. Observe the log output.

Sample output
=============

On success, the sample prints messages similar to the following:

.. code-block:: console

   <inf> fwup: Reset the chip
   <inf> fwup: Reset done
   <inf> fwup: Chip in bootloader mode
   <inf> fwup: Start flash erase
   <inf> fwup: Flash erase done
   <inf> fwup: Start flashing firmware (61320 bytes)
   <inf> fwup: Flashing done
   <inf> fwup: Rebooting
   <inf> fwup: Reboot done
   <inf> fwup: Chip in transceiver mode
   <inf> application: Expected firmware running
   <inf> application: Flash another application (for example sid_end_device) to continue

If the update fails, power-cycle the board and run the sample again.

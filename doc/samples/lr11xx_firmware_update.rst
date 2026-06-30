.. _lr11xx_firmware_update:

LR11xx firmware update
######################

.. contents::
   :local:
   :depth: 2

This sample updates the Semtech LR1110 transceiver firmware to version ``0x0401``, which is the minimum version required by Sidewalk.
Use it when the LR1110 module runs older firmware and Sidewalk sub-GHz samples fail during radio initialization.
After a successful update, flash a Sidewalk application such as :ref:`sidewalk_end_device`.

Requirements
************

This sample supports the following development kits:

.. list-table::
   :header-rows: 1

   * - Hardware platforms
     - PCA
     - Board name
     - Build target
   * - `nRF54L15 DK <nRF54L15 Product Page_>`_
     - PCA10156
     - `nrf54l15dk`_
     - ``nrf54l15dk/nrf54l15/cpuapp``
   * - `nRF54LM20 DK <nRF54LM20A Product Page_>`_
     - PCA10184
     - `nrf54lm20dk`_
     - | ``nrf54lm20dk/nrf54lm20a/cpuapp``
       | ``nrf54lm20dk/nrf54lm20b/cpuapp``

You also need one of the following LR1110 radio boards:

* `Semtech LR1110MB1LCKS`_ Arduino shield with the ``simple_arduino_adapter`` shield on the nRF54L15 DK
* :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` expansion board on the nRF54L15 DK or nRF54LM20 DK

For hardware setup and pin assignment, refer to :ref:`setting_up_hardware`.

Overview
********

The sample initializes the Sidewalk radio platform, resets the LR1110 into bootloader mode, erases flash, writes the embedded transceiver firmware image, and reboots the chip.
It then verifies that the transceiver reports the expected firmware version.

The sample does not start the Sidewalk stack or connect to a gateway.
Run it once before flashing a full Sidewalk application.

The LR1110 firmware images are available in the `LR11xx firmware images repository`_.
The sample already includes the ``0x0401`` firmware image.

Building and running
********************

This sample can be found under :file:`samples/lr11xx_firmware_update`.

For general instructions on building the sample, follow the steps in the `Building and programming an application`_ documentation.

Build and flash the sample for your board and shield combination using the following commands.

Semtech LR1110 Arduino shield
=============================

.. tabs::

   .. tab:: nrf54l15dk/nrf54l15/cpuapp

      .. code-block:: console

         west build -b nrf54l15dk/nrf54l15/cpuapp --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs
         west flash

nRF Sidewalk EB shield
======================

.. tabs::

   .. tab:: nrf54l15dk/nrf54l15/cpuapp

      .. code-block:: console

         west build -b nrf54l15dk/nrf54l15/cpuapp --shield nrf_sidewalk_eb
         west flash

   .. tab:: nrf54lm20dk/nrf54lm20a/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20a/cpuapp --shield nrf_sidewalk_eb
         west flash

   .. tab:: nrf54lm20dk/nrf54lm20b/cpuapp

      .. code-block:: console

         west build -b nrf54lm20dk/nrf54lm20b/cpuapp --shield nrf_sidewalk_eb
         west flash

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After building and flashing the sample, complete the following steps to verify the update:

#. Connect the development kit to your PC using a USB cable.
#. Connect the LR1110 radio board to the development kit.
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

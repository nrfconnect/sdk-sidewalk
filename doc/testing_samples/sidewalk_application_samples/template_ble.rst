.. _template_ble:

Template Bluetooth LE
#####################

The sample demonstrates a template for Sidewalk End Node application.
It is optimized for Bluetooth LE.

.. _template_ble_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

.. _template_ble_overview:

Overview
********

The sample shows implementation of the Sidewalk API for the Bluetooth LE transport protocol.
It is a memory-optimized example of Sidewalk configuration where only Bluetooth LE transport protocol is supported.
Because of the smaller footprint, both bootloader partitions for the application are placed in the internal flash memory of the supported SoC (nRF52840).

.. _template_ble_ui:

User interface
**************

A button action is triggered when you release the button.
To use a long press action, hold a button for 2 seconds or longer, and release it.

The button assignment is as follows:

   * Button 1 (long press):
      Factory Reset - The application informs the Sidewalk stack about the factory reset event.
      The Sidewalk library clears its configuration from the non-volatile storage.
      After a successful reset, the device needs to be registered with the cloud services again.

   * Button 2:
      Toggle Connection Request - The device requests the Sidewalk Gateway to initiate a connection while the device is advertising through Bluetooth LE.
      After the connection is dropped, the user has to set the beacon state again.
      Gateways may not always be able to process this request, as it depends on the number of devices connected to it.

   * Button 3:
      Send Hello - This action will queue a message to the cloud.
      If Sidewalk is not ready, it will simply show an error without sending the message.
      The queue will be processed eventually, and all the queued messages will be sent.

   * Button 4 (short press):
      Set fake battery level - The action sets a simulated battery level.

   * Button 4 (long press):
      Enter DFU state - This action disables the Sidewalk stack and starts the Bluetooth LE SMP Server.
      You can update the firmware image using nRF Connect for mobile application.
      To exit DFU state, perform a power cycle on your device.


LEDs represent the current state of the application:

   * LED 1:
      Application ``Connected`` successfully.

   * LED 2:
      Application ``Registered`` successfully.

   * LED 3:
      Application ``time sync`` successful.

   * LED 4:
      Application ``link`` is up.

Other states are routed to generic GPIO pins.
When the voltage drops to 0, entering low logic state, it means that one of the following states is active:

   * P1.1:
      ``DFU`` state.

   * P1.2:
      ``ERROR`` has occured.

   * P1.3:
      ``Sending`` pin is active when the packet is being transmitted.

   * P1.4:
      The ``Received`` pin is toggled ``on`` and immediately ``off`` when the packet has been received.

.. _template_ble_config:

Configuration
*************

For this sample, you can use the following overlays:

* :file:`cli` - This configuration adds an nRF Connect Shell over with Sidewalk Command Line Interface.

* :file:`usb_dfu` - This configuration adds a USB device to the application which acts as the target for the firmware update.
  A new image is then acquired through USB and placed in the second partition for the bootloader.
  For more details, see the `USB DFU Sample Application`_ documentation.

Bootloader and Device Firmware Update (DFU)
*******************************************

MCUboot is configured to have two banks of memory for an application.

For more information on bootloaders and delivery options for the updated images, see `Bootloader and DFU solutions for NCS`_.

Memory layout
=============

The following table presents how the memory is allocated for the Template Bluetooth LE sample:

    .. code::

        flash_primary (0x00000000 - 0x000fffff):
      +-------------------------------------------------------+
      | 0x00000000 - 0x00006fff: mcuboot (28kB)               |
      +---mcuboot_primary (480kB)-----------------------------+
      | 0x00007000 - 0x000071ff: mcuboot_pad (512B)           |
      | 0x00072000 - 0x0007efff: mcuboot_primary_app (479.5kB)|
      +-------------------------------------------------------+
      | 0x0007f000 - 0x000f6fff: mcuboot_secondary (480kB)    |
      | 0x000f7000 - 0x000f8fff: settings_storage (8kB)       |
      | 0x000f9000 - 0x000fefff: sidewalk_storage (24kB)      |
      | 0x000ff000 - 0x000fffff: mfg_storage (4kB)            |
      +-------------------------------------------------------+

        sram_primary (0x20000000 - 0x2003ffff):
      +-------------------------------------------------------+
      | 0x20000000 - 0x2003ffff: sram_primary (256kB)         |
      +-------------------------------------------------------+

DFU services
============

If a new, correct image is found, the bootloader can only start the application placed in the first partition and swap the application from the second partition.
It cannot take any new image other than the one from the secondary partition.

Building and running
********************

This sample can be found under :file:`samples/template_ble`.


#. If you are building the sample for the first time, set up the environment:

   .. note::
      This step needs to be completed only once.
      You do not have to repeat it on every sample rebuild.

   a. Run the application manifest and update.

      .. code-block:: console

         $ west config manifest.path sidewalk
         $ west update

   #. Flash the :file:`mfg.hex` file.

      .. code-block:: console

         $ nrfjprog --chiperase --family NRF52 \
         --program ${ZEPHYR_BASE}/../sidewalk/tools/provision/mfg.hex \
         --reset

#. Once you have set up your environment, build the sample.
   Depending on the configuration, build with one of the commands below:

   * Build with the default configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840

   * Build with the release configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf

   * Build with the debug configuration with CLI

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DCONFIG_SIDEWALK_CLI=y

   .. note::
      For more details on building with west, see the `West building flashing and debugging`_ documentation.

#. Flash the Sidewalk application.

   .. code-block:: console

	   $ west flash

#. Confirm the Sidewalk sample is flashed.

   a. List a serial port of the connected development kit.

      .. code-block:: console

          $ nrfjprog --com
          683929577    /dev/ttyACM0    VCOM0

   #. Connect to the listed serial port with the following settings:

      * Baud rate: 115200
      * 8 data bits
      * 1 stop bit
      * No parity
      * HW flow control: None

      For example, you can use the Screen tool:

      .. code-block:: console

        $  screen  /dev/ttyACM0 115200

   #. To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.

      .. code-block:: console

         *** Booting Zephyr OS build v3.0.99-ncs1-4913-gf7b06162027d  ***
         ----------------------------------------------------------------
         sidewalk             v1.13.0-6-g2e0691d-dirty
         nrf                  v2.0.0-734-g3904875f6
         zephyr               v3.0.99-ncs1-4913-gf7b0616202-dirty
         ----------------------------------------------------------------
         sidewalk_fork_point = 92dcbff2da68dc6853412de792c06cc6966b8b79
         build time          = 2022-11-17 10:52:50.833532+00:00
         ----------------------------------------------------------------

         [00:00:00.006,225] <inf> sid_template: Sidewalk example started!

.. _template_ble_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device, follow the steps in :ref:`sidewalk_testing`.

.. _USB DFU Sample Application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/samples/subsys/usb/dfu/README.html
.. _Bootloader and DFU solutions for NCS: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/app_bootloaders.html
.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html
.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832
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

To test the Device Firmware Update, follow :ref:`sidewalk_testing_dfu`.

If a new, correct image is found, the bootloader can only start the application that is placed in the first partition and exchange the application from the second partition.
It may take a few seconds.

Building and running
********************

This sample can be found under :file:`samples/template_ble`.

.. note::
   Before you flash you Sidewalk sample, make sure you have:

      * Downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * Provisioned your device during the :ref:`setting_up_sidewalk_product`

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type, see :ref:`template_ble_selecting_build`.

.. _template_ble_selecting_build:

Selecting a build type
======================

Use one of the common sample configurations:

   * Build with the default configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840

   * Build with the release configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf

   * Build with the debug configuration with CLI

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DCONFIG_SIDEWALK_CLI=y

.. _template_ble_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.
You should see the following output:

      .. code-block:: console

         *** Booting Zephyr OS build v3.2.99-ncs2 ***
         ----------------------------------------------------------------
         sidewalk             v1.14.3-1-g1232aabb
         nrf                  v2.3.0
         zephyr               v3.2.99-ncs2
         ----------------------------------------------------------------
         sidewalk_fork_point = af5d608303eb03465f35e369ef22ad6c02564ac6
         build time          = 2023-03-14 15:00:00.000000+00:00
         ----------------------------------------------------------------

         [00:00:00.006,225] <inf> sid_template: Sidewalk example started!

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

To test the device, follow the steps in :ref:`sidewalk_testing`.

.. _USB DFU Sample Application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/samples/subsys/usb/dfu/README.html
.. _Bootloader and DFU solutions for NCS: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/app_bootloaders.html
.. _Building and programming an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/programming.html#gs-programming
.. _Testing and debugging an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/testing.html#gs-testing
.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832
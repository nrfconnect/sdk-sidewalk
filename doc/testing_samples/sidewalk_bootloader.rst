.. _bootloader_and_dfu_for_sidewalk:

Bootloader and Device Firmware Update (DFU) for Sidewalk
########################################################

In Sidewalk, every sample is built with the MCUBoot bootloader.
The MCUBoot is configured to have two banks of memory for an application.

For more information on bootloaders and delivery options for the updated images, see the `Bootloader and DFU solutions for NCS`_ documentation.

Configuration in samples
************************

The following configurations are available for Sidewalk:

* The ble_only sample - It is an example of Sidewalk configuration where only Bluetooth LE transport protocol is supported.
  Because of the smaller footprint, both bootloader partitions for the application are placed in the internal flash memory of the supported SoC (nRF52840).

* The template sample - It supports all available configurations (including ble_only), but because the size and location of the bootloader partitions have to be consistent, the sample uses an external memory to store the secondary partition for the application.
  Partitions are set in the bootloader, and their adequate size is reservered.
  This way, you should be able to update any template configuration or buy any other configuration.


Memory layout
*************

To see how the memory is allocated for the BLE-only and template samples, refer to the table below:

.. tabs::

  .. group-tab:: BLE-only

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

  .. group-tab:: Template
     
    .. code::

        external_flash (0x00000000 - 0x007fffff):
      +-------------------------------------------------------+
      | 0x00000000 - 0x000eefff: mcuboot_secondary (956kB)    |
      | 0x000ef000 - 0x007fffff: external_flash (7.07MB)      |
      +-------------------------------------------------------+

        flash_primary (0x00000000 - 0x000fffff):
      +-------------------------------------------------------+
      | 0x00000000 - 0x00006fff: mcuboot (28kB)               |
      +---mcuboot_primary (480kB)-----------------------------+
      | 0x00007000 - 0x000071ff: mcuboot_pad (512B)           |
      | 0x00072000 - 0x000f5fff: mcuboot_primary_app (955.5kB)|
      +-------------------------------------------------------+
      | 0x000f6000 - 0x000f7fff: settings_storage (8kB)       |
      | 0x000f8000 - 0x000fefff: sidewalk_storage (28kB)      |
      | 0x000ff000 - 0x000fffff: mfg_storage (4kB)            |
      +-------------------------------------------------------+

        sram_primary (0x20000000 - 0x2003ffff):
      +-------------------------------------------------------+
      | 0x20000000 - 0x2003ffff: sram_primary (256kB)         |
      +-------------------------------------------------------+


DFU Services
************

If a new, correct image is found, the bootloader will only be able to start an application placed in the first partition and swap the application from the second partition.
It does not have a capability to take any new image other than the one from the secondary partition.

By default, an application also does not have a capability to provide a new image to the secondary partition.

USB Service
-----------

To enable the DFU service in your application, use the :file:`usb_dfu.conf` overlay for building.
It creates a USB device that, once connected to the PC, can interact with the dfu-util to send an update.

.. note::

    For DFU, use the USB port located on the side of your development kit, as it is directly connected to the SoC.
    The USB port located on the edge of the board is used only for JLink and passing UART.

For instructions on how to use this method for a firmware update, see the Zephyr USB DFU sample in `USB DFU Sample Application`_ documentation.

.. _Bootloader and DFU solutions for NCS: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/app_bootloaders.html
.. _MCUBoot: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/mcuboot/index-ncs.html
.. _USB DFU Sample Application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/samples/subsys/usb/dfu/README.html

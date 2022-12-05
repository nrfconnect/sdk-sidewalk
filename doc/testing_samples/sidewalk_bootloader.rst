.. _bootloader_and_dfu_for_sidewalk:

Bootloader and DFU for Sidewalk
###############################

Bootloader
**********

Every sample of Sidewalk application gets build with `MCUBoot`_ as a `Bootloader and DFU solutions for NCS`_.

MCUBoot is configured to have two banks of memory for the application.

Configuration in Samples
************************

**The ble_only sample** is an example of Sidewalk configuration where only ble transport protocol is supported, and because of the smaller footprint, both bootloader partitions for application are placed in the internal flash memory of the supported SoC (nrf52840).

**The template sample** supports all available configurations (including ble_only) but because the size and location of the bootloader partitions have to be consistent, this sample uses external memory to store secondary partition for the application.
We want to be able to update any template configuration, buy any other configuration, so we have to reserve adequate size of the partitions which are set in bootloader. 


DFU Services
************

The bootloader is only responsible for starting the application placed in first partition, and for swapping the application from second partition, if new, correct image has been found there. It does not have any other capability to take the new image other than the secondary partition.

By default application also does not have any capability to provide the new image to the secondary partition.

**USB Service**
To enable the DFU service in the application, use the usb_dfu.conf overlay for building. It creates an usb device that connected to the PC can interact with the dfu-util to send the update.

.. note:: 

    For DFU use usb port located on the side of the development kit - it is directly connected to the SoC.
    Usb on the edge of board is used only for JLink (and passing UART, but it is feature of JLink).

Check Zephyr usb dfu sample for instructions how to use this method for firmware update: https://docs.zephyrproject.org/2.6.0/samples/subsys/usb/dfu/README.html


.. _Bootloader and DFU solutions for NCS : https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/app_bootloaders.html
.. _MCUBoot: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/mcuboot/index-ncs.html

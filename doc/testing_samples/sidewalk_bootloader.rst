.. _bootloader_and_dfu_for_sidewalk:

Bootloader and DFU for Sidewalk
###############################

Every sample of Sidewalk application gets build with MCUBoot as a bootloader.

MCUBoot is configured to have two banks of memory for the application.  

The sample ble_only is an example of Sidewalk configuration where only ble transport protocol is supported, and because of the smaller footrint, both bootloader partitions for application are placed in the internal flash memory of the supported SoC (nrf52840).

The template sample supports all avaliable configurations (including ble_only) but because the size and location of the bootloader paritions have to be consistant, this sample uses external memory to store secondary partition for the application.
We want to be able to update any template configuration, buy any other configuration, so we have to reserve adequate size of the partitions which are set in bootloader. 


DFU
###

The bootloader is only responsible for starting the application placed in first partition, and for swaping the application from second partition, if new, correct image has been found there. It does not have any other capabity to take the new image other than the secondary partition.

By default application also does not have any capability to provide the new image to the secondary partition.
To enable the DFU service in the application, use the usb_dfu.conf overlay for building. It creates an usb device that connected to the PC can interract with the dfu-util to send the update.

.. note:: 

    For DFU use usb port located on the side of the development kit - it is directly connected to the SoC.
    Usb on the edge of board is used only for Jlink (and passing uart, but it is feature of Jlink).

Check Zephyr usb dfu sample for instructions how to use this method for firmware update: https://docs.zephyrproject.org/2.6.0/samples/subsys/usb/dfu/README.html

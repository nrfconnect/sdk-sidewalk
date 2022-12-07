.. _testing_sidewalk_samples:

Structure of samples in Sidewalk project
****************************************

In the Sidewalk repository, samples are placed in :file:`samples` directory.
Currently there are two samples:

 - :file:`template`
 - :file:`ble_only`

 Table below shows the difference between those configurations:

+-----------------------+-----------------------------------------------------------+--------------------------------------------+
|                       | template                                                  | ble_only                                   |
+=======================+===========================================================+============================================+
| Hardware Required     | nrf52840dk_nrf52840                                       | nrf52840dk_nrf52840                        |
|                       |                                                           |                                            |
|                       | external flash (included in DK)                           |                                            |
|                       |                                                           |                                            |
|                       | Semtech sx1262 (only for configurations with LoRa / FSK)  |                                            |
+-----------------------+-----------------------------------------------------------+--------------------------------------------+
| Bootloader            | MCUBoot - DFU partition in external flash                 | MCUBoot - DFU partition in internal flash  |
|                       |                                                           |                                            |
|                       | Application partition size = 984kB                        | Application partition size = 492kB         |
+-----------------------+-----------------------------------------------------------+--------------------------------------------+
| Application overlays  | lora                                                      | usb_dfu                                    |
|                       |                                                           |                                            |
|                       | fsk                                                       |                                            |
|                       |                                                           |                                            |
|                       | usb_dfu                                                   |                                            |
+-----------------------+-----------------------------------------------------------+--------------------------------------------+


Overlays overview:

- :file:`lora.conf` - This configuration enables LoRa transport in Sidewalk application. Enabeling This overlay requires the Semtech sx1262 shield to be connected.

- :file:`fsk.conf` - This configuration enables FSK transport in Sidewalk application. Enabeling This overlay requires the Semtech sx1262 shield to be connected.

- :file:`usb_dfu` - This adds USB device to the application that acts as target for firmware update (It takes new image via USB and places it in second partition for bootloader)


Testing samples
***************

For the exact instructions on building and testing Sidewalk samples, see the documentation below:

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   building_samples.rst
   sidewalk_bootloader.rst
   sidewalk_registration.rst
   sidewalk_testing.rst
   sidewalk_testing_with_cli.rst
   sidewalk_troubleshooting.rst

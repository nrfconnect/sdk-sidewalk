.. _testing_sidewalk_samples:

Samples structure in Sidewalk
*****************************

In the Sidewalk repository, samples are placed in the :file:`samples` directory.
Currently, there are two samples:

 * :file:`template`
 * :file:`ble_only`

The table below shows differences between their configurations:

+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Criteria              | Template                                                      | BLE only                                   |
+=======================+===============================================================+============================================+
| Hardware required     | * `nRF52840dk_nrf52840`_                                      | * `nRF52840dk_nrf52840`_                   |
|                       | * External flash (included in the DK)                         |                                            |
|                       | * Semtech sx1262 (only for the configurations with LoRa/FSK)  |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Bootloader            | * MCUBoot - DFU partition in external flash                   | * MCUBoot - DFU partition in internal flash|
|                       | * Application partition size = 984kB                          | * Application partition size = 492kB       |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Application overlays  | * :file:`lora.conf`                                           | file:`usb_dfu`                             |
|                       | * :file:`fsk.conf`                                            |                                            |
|                       | * :file:`usb_dfu`                                             |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+

Overlays overview

- :file:`lora.conf` - This configuration enables LoRa transport in the Sidewalk application.
  To enable this overlay you have to connect the Semtech sx1262 shield.

- :file:`fsk.conf` - This configuration enables FSK transport in the Sidewalk application.
  To enable this overlay you have to connect the Semtech sx1262 shield.

- :file:`usb_dfu` - This file adds a USB device to the application which acts as the target for the firmware update.
  A new image is then taken through the USB and placed in the second partition for bootloader.


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

.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832

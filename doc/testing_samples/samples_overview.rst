.. _samples_overview:

Samples overview
################

.. note::
    For the best user experience, start your work with the Sensor Monitoring application.

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.
The following table outlines the differences in their configurations:

+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| Sample                      | Hardware required                                                        | Bootloader                                    | Application overlays                        | Persistant data size                | Supported transports     |
+=============================+==========================================================================+===============================================+=============================================+=====================================+==========================+
| :ref:`sensor_monitoring`    | * `nRF52840dk_nrf52840`_                                                 | --                                            | * :file:`fsk.conf`                          | * 8 kB Zephyr settings              | * Bluetooth LE           |
|                             | * `Semtech SX1262MB2CAS`_                                                |                                               | * :file:`lora.conf`                         | * 24 kB Sidewalk key-value storage  | * LoRa                   |
|                             |                                                                          |                                               |                                             |                                     | * FSK                    |
+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| :ref:`template_subghz`      | * `nRF52840dk_nrf52840`_                                                 | * MCUboot - DFU partition in external flash   | * :file:`fsk.conf`                          | * 8 kB Zephyr settings              | * LoRa                   |
|                             | * External flash (included in the DK)                                    | * Application partition size = 956 kB         | * :file:`usb_dfu`                           | * 28 kB Sidewalk key-value storage  | * FSK                    |
|                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) |                                               |                                             |                                     |                          |
+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| :ref:`template_ble`         | * `nRF52840dk_nrf52840`_                                                 | * MCUboot - DFU partition in internal flash   | * :file:`usb_dfu`                           | * 8kB Zephyr settings               | * Bluetooth LE           |
|                             |                                                                          | * Application partition size = 480 kB         |                                             | * 24 kB Sidewalk key-value storage  |                          |
+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| :ref:`device_under_test`    | * `nRF52840dk_nrf52840`_                                                 | --                                            | --                                          | * 8 kB Zephyr settings              | * Bluetooth LE           |
|                             | * `Semtech SX1262MB2CAS`_                                                |                                               |                                             | * 24 kB Sidewalk key-value storage  | * LoRa                   |
|                             |                                                                          |                                               |                                             |                                     | * FSK                    |
+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+

.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/boards/arm/nrf52840dk_nrf52840/doc/index.html
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
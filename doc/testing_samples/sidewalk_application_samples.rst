.. _sidewalk_samples:

Sidewalk application samples
############################

In the Sidewalk repository, samples are placed in the :file:`samples` directory.

To learn more about the samples, refer to the subpages below.
For information about building the sample, see the :ref:`samples_building_and_running` documentation.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   sidewalk_application_samples/ble_only.rst
   sidewalk_application_samples/template.rst

The table below shows differences between their configurations:

+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Criteria              | Template                                                      | Bluetooth LE only                          |
+=======================+===============================================================+============================================+
| Hardware required     | * `nRF52840dk_nrf52840`_                                      | * `nRF52840dk_nrf52840`_                   |
|                       | * External flash (included in the DK)                         |                                            |
|                       | * Semtech sx1262 (only for the configurations with LoRa/FSK)  |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Bootloader            | * MCUBoot - DFU partition in external flash                   | * MCUBoot - DFU partition in internal flash|
|                       | * Application partition size = 984kB                          | * Application partition size = 492kB       |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Application overlays  | * :file:`lora.conf`                                           | * :file:`usb_dfu`                          |
|                       | * :file:`fsk.conf`                                            |                                            |
|                       | * :file:`usb_dfu`                                             |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+

.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832

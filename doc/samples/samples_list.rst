.. _samples_list:

Samples overview
################

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The current sample structure includes the :ref:`sidewalk_end_device` sample with available variants.
The following table demonstrates the variants' configuration for the supported development kits:

+-----------------------------------+------------------------------------------+--------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| Sample variant                    | Description                              | Development kit                | Additional hardware                                                       | Bootloader (in release and debug configs)     | Application overlays      |
+===================================+==========================================+================================+===========================================================================+===============================================+===========================+
| :ref:`variant_sidewalk_hello`     | Sample application showing communication | * `nRF52840 DK`_               | * External flash (included in the DK) except `nRF54L15 PDK`_              | * MCUboot - DFU partition in external flash   | * ``overlay-hello.conf``  |
|                                   | over Sidewalk network.                   | * `nRF5340 DK`_                | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK)  | * Application partition size = 956 kB         |                           |
|                                   |                                          | * `nRF54L15 PDK`_ `*`          |                                                                           |                                               |                           |
+-----------------------------------+------------------------------------------+--------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| :ref:`variant_sensor_monitoring`  | Demo application of Sidewalk protocol    | * `nRF52840 DK`_               | * External flash (included in the DK) except `nRF54L15 PDK`_              | * MCUboot - DFU partition in external flash   | * ``overlay-demo.conf``   |
|                                   | with  temperature sensor monitoring.     | * `nRF5340 DK`_                | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK)  | * Application partition size = 956 kB         |                           |
|                                   |                                          | * `Thingy53`_                  |                                                                           |                                               |                           |
|                                   |                                          | * `nRF54L15 PDK`_ `*`          |                                                                           |                                               |                           |
+-----------------------------------+------------------------------------------+--------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| :ref:`variant_sidewalk_dut`       | Device under test generic application    | * `nRF52840 DK`_               | * External flash (included in the DK) except `nRF54L15 PDK`_              | * MCUboot - DFU partition in external flash   | * ``overlay-dut.conf``    |
|                                   | having activated CLI.                    | * `nRF5340 DK`_                | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK)  | * Application partition size = 956 kB         |                           |
|                                   |                                          | * `nRF54L15 PDK`_ `*`          |                                                                           |                                               |                           |
+-----------------------------------+------------------------------------------+--------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+

`*` The `nRF54L15 PDK`_ does not support LoRa and FSK configurations.

.. note::

   With the v2.6.0 release of the `nRF Connect SDK`_ a new sample model has been introduced.
   To see how the changes might affect your application, see the :ref:`migration_guide`.

.. toctree::
   :maxdepth: 3
   :glob:
   :caption: Subpages:

   sid_end_device.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

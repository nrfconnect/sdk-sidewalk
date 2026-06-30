.. _samples_list:

Samples overview
################

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The repository includes two samples:

* :ref:`sidewalk_end_device` — The main Sidewalk application, available in several variants.
* :ref:`lr11xx_firmware_update` — A utility sample for updating Semtech LR1110 transceiver firmware before running sub-GHz Sidewalk applications.

Sidewalk end device
*******************

The :ref:`sidewalk_end_device` sample is the primary Sidewalk application in this repository.
It demonstrates end-to-end Sidewalk functionality and can be built with different overlays depending on your use case.

The following table summarizes the supported variants, development kits, and build configuration for this sample only:

+-----------------------------------+------------------------------------------+-------------------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| Sample variant                    | Description                              | Development kit                           | Additional hardware                                                       | Bootloader (in release and debug configs)     | Application overlays      |
+===================================+==========================================+===========================================+===========================================================================+===============================================+===========================+
| :ref:`variant_sidewalk_hello`     | Sample application showing communication | * `nRF54L15 DK`_                          | * External flash (included in the DK) except `nRF54L15 DK`_               | * MCUboot - DFU partition in external flash   | * ``overlay-hello.conf``  |
|                                   | over Sidewalk network.                   | * nRF54L10 emulating on `nRF54L15 DK`_    | * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)                 | * Application partition size = 956 kB         |                           |
|                                   |                                          | * `nRF54LV10 DK`_                         |                                                                           |                                               |                           |
|                                   |                                          | * `nRF54LM20 DK`_                         |                                                                           |                                               |                           |
+-----------------------------------+------------------------------------------+-------------------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| :ref:`variant_sensor_monitoring`  | Demo application of Sidewalk protocol    | * `nRF54L15 DK`_                          | * External flash (included in the DK) except `nRF54L15 DK`_               | * MCUboot - DFU partition in external flash   | * ``overlay-demo.conf``   |
|                                   | with temperature sensor monitoring.      | * nRF54L10 emulating on `nRF54L15 DK`_    | * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)                 | * Application partition size = 956 kB         |                           |
+-----------------------------------+------------------------------------------+-------------------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+
| :ref:`variant_sidewalk_dut`       | Device under test generic application    | * `nRF54L15 DK`_                          | * External flash (included in the DK) except `nRF54L15 DK`_               | * MCUboot - DFU partition in external flash   | * ``overlay-dut.conf``    |
|                                   | with CLI enabled.                        | * nRF54L10 emulating on `nRF54L15 DK`_    | * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)                 | * Application partition size = 956 kB         |                           |
|                                   |                                          | * `nRF54LV10 DK`_                         |                                                                           |                                               |                           |
|                                   |                                          | * `nRF54LM20 DK`_                         |                                                                           |                                               |                           |
+-----------------------------------+------------------------------------------+-------------------------------------------+---------------------------------------------------------------------------+-----------------------------------------------+---------------------------+

For variant-specific build, test, and configuration details, see :ref:`sidewalk_end_device`.

.. toctree::
   :maxdepth: 3
   :glob:
   :caption: Subpages:

   lr11xx_firmware_update.rst
   sid_end_device.rst

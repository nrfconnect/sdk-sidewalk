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

   Targets `nRF5340 DK`_  and `Thingy53`_  use `MCUBOOT_MODE_OVERWRITE_ONLY` configuration for bootloader. This means, that after the DFU image swap, there is no rollback mechanism in case new image is not functional.
   This limitation is due to split between app and net core. If the default MCUBOOT mode to be set, the rollback would only apply to the app core, and not to the net core. This could lead to missmatch between app and net core.
   Please verify the image to be updated before performing DFU on large scale as faulty image may lead to unrecoverable state of the device. 
   Other platforms use default MCUBOOT mode (`MCUBOOT_MODE_SWAP_WITHOUT_SCRATCH`), and it requires new image to be confirmed (either externally, or by the new application itself), otherwise on next reboot mcuboot will revert the change. 

.. note::

   With the v2.6.0 release of the `nRF Connect SDK`_ a new sample model has been introduced.
   To see how the changes might affect your application, see the :ref:`migration_guide_v260`.

.. toctree::
   :maxdepth: 3
   :glob:
   :caption: Subpages:

   sid_end_device.rst

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

.. list-table::
   :header-rows: 1
   :widths: 14 18 16 28 22 12

   * - Sample variant
     - Description
     - Development kit
     - Additional hardware
     - Bootloader (in release and debug configs)
     - Application overlays
   * - :ref:`variant_sidewalk_hello`
     - Sample application showing communication over Sidewalk network.
     - * `nRF52840 DK`_
       * `nRF54L15 DK`_
       * nRF54L10 emulated on `nRF54L15 DK`_
       * `nRF54LV10 DK`_
       * `nRF54LM20 DK`_
     - * External flash (included in the DK) on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
       * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)
       * :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` on `nRF54LM20 DK`_ (LoRa/FSK)
     - MCUboot — DFU partition in external flash on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
     - ``overlay-hello.conf``
   * - :ref:`variant_sensor_monitoring`
     - Demo application of Sidewalk protocol with temperature sensor monitoring.
     - * `nRF52840 DK`_
       * `nRF54L15 DK`_
       * nRF54L10 emulated on `nRF54L15 DK`_
     - * External flash (included in the DK) on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
       * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)
     - MCUboot — DFU partition in external flash on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
     - ``overlay-demo.conf``
   * - :ref:`variant_sidewalk_dut`
     - Device under test generic application with CLI enabled.
     - * `nRF52840 DK`_
       * `nRF54L15 DK`_
       * nRF54L10 emulated on `nRF54L15 DK`_
       * `nRF54LV10 DK`_
       * `nRF54LM20 DK`_
     - * External flash (included in the DK) on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
       * `Semtech SX1262MB2CAS`_ or LR1110MB1xxS (LoRa/FSK only)
       * :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` on `nRF54LM20 DK`_ (LoRa/FSK)
       * `nRF7002 EB II`_ on `nRF54L15 DK`_ and `nRF54LM20 DK`_ (Wi-Fi location)
     - MCUboot — DFU partition in external flash on `nRF52840 DK`_ and nRF54L10 emulated on `nRF54L15 DK`_
     - ``overlay-dut.conf``

For variant-specific build, test, and configuration details, see :ref:`sidewalk_end_device`.

.. toctree::
   :maxdepth: 3
   :glob:
   :caption: Subpages:

   lr11xx_firmware_update.rst
   sid_end_device.rst

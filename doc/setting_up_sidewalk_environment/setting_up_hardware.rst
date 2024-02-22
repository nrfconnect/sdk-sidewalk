.. _setting_up_hardware:

Setting up hardware
###################

.. contents::
   :local:
   :depth: 2

Before setting up the Sidewalk environment, make sure you have correctly set up the required hardware.

Development kit
***************

Nordic Semiconductor's development kit is required to run Sidewalk samples.
For the list of supported DKs per sample, see the :ref:`samples_list`.

Depending on the device, refer to the getting started guide for setup instructions:

* `Getting started with nRF52 Series`_
* `Getting started with nRF53 Series`_

Semtech SX1262
**************

Sidewalk samples use sub-GHz radio transceivers to support LoRa and (G)FSK modulation.
You will need it to run :ref:`LoRa and FSK applications <samples_list>`.

For evaluation, use the `Semtech SX1262MB2CAS`_ Arduino Shield.

.. figure:: /images/nRF52840-Semtech-SX1262.jpg

.. _setting_up_hardware_semtech_pinout:

Pinout
------

Connect the Semtech shield to the Arduino-compatible headers on the development kit.


.. tabs::

   .. tab:: nRF52840

    Refer to the pinout assignment for nRF52840:

    +---------------+-------------+
    | SX126X shield | nRF52840 DK |
    +===============+=============+
    | BUSY          | P1.04       |
    +---------------+-------------+
    | DIO1          | P1.06       |
    +---------------+-------------+
    | NSS           | P1.08       |
    +---------------+-------------+
    | ANT_SW        | P1.10       |
    +---------------+-------------+
    | SPI MOSI      | P1.13       |
    +---------------+-------------+
    | SPI MISO      | P1.14       |
    +---------------+-------------+
    | SPI SCK       | P1.15       |
    +---------------+-------------+
    | NRESET        | P0.03       |
    +---------------+-------------+

   .. tab:: nRF5340

    Refer to the pinout assignment for nRF5340:

    +---------------+-------------+
    | SX126X shield | nRF5340 DK  |
    +===============+=============+
    | BUSY          | P1.5        |
    +---------------+-------------+
    | DIO1          | P1.7        |
    +---------------+-------------+
    | NSS           | P1.9        |
    +---------------+-------------+
    | ANT_SW        | P1.10       |
    +---------------+-------------+
    | SPI MOSI      | P1.13       |
    +---------------+-------------+
    | SPI MISO      | P1.14       |
    +---------------+-------------+
    | SPI SCK       | P1.15       |
    +---------------+-------------+
    | NRESET        | P0.04       |
    +---------------+-------------+

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

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
* `Getting started with nRF54L Series`_

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

   .. tab:: nRF52840 DK

    Refer to the pinout assignment for the nRF52840 DK:

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

   .. tab:: nRF5340 DK

    Refer to the pinout assignment for the nRF5340 DK:

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

.. _setting_up_sidewalk_gateway:

Sidewalk gateway
****************

.. note::
   Refer to the `Amazon Sidewalk gateways`_ for the complete list of available devices.

For the exact instructions, refer to the `Setting up a Sidewalk gateway`_ documentation.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

.. include:: ../ncs_links.rst

.. _Echo: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id94401162
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app
.. _Amazon Sidewalk gateways: https://docs.sidewalk.amazon/introduction/sidewalk-gateways.html
.. _Setting up a Sidewalk gateway: https://docs.sidewalk.amazon/getting-started/sidewalk-onboard-prereq-gateway.html
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

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
    | BUSY          | P1.4        |
    +---------------+-------------+
    | DIO1          | P1.6        |
    +---------------+-------------+
    | NSS           | P1.8        |
    +---------------+-------------+
    | ANT_SW        | P1.10       |
    +---------------+-------------+
    | SPI MOSI      | P1.13       |
    +---------------+-------------+
    | SPI MISO      | P1.14       |
    +---------------+-------------+
    | SPI SCK       | P1.15       |
    +---------------+-------------+
    | NRESET        | P0.3        |
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
    | NRESET        | P0.4        |
    +---------------+-------------+

   .. tab:: nRF54L15 PDK

      The nRF54L15 PDK does not have the Arduino-compatible header.
      The connection layout may differ depending on the revision of your PDK.
      GPIO assignments are recommended by the Nordic Semiconductor.

      The nRF54L15 PDK operates on a lower voltage level (1.8V).
      The Semtech Shield is compatible with this voltage and can be safely provided to the pin marked as 3.3V.
      Due to the lower voltage, the Semtech radio cannot achieve the full transmit power, and it will reach a plateau at 15 dBm gain even when a higher value is configured.
      The specification of the Semtech module describes that it is possible to provide a separate, higher voltage source for radio while maintaining the lower voltage for communication.
      However, the recommended shield does not support this configuration.

      To use the suggested pins, disable **UART0** (not used by Sidewalk samples) through the `Board Configurator`_ tool in the nRF Connect for Desktop.
      This step is required for the shield to work as some pins are connected to **UART0** by default.

      See a board configuration for the suggested pin layout:

      +--------------------------------------------------------------+
      |                                                              |
      | .. figure:: /images/board_configurator_nrf54l15_0_3_0.png    |
      |                                                              |
      +--------------------------------------------------------------+

      +---------------+--------------+
      | SX126X shield | nRF54L15 DK  |
      +===============+==============+
      | DIO1          | P1.11        |
      +---------------+--------------+
      | BUSY          | P2.6         |
      +---------------+--------------+
      | NRESET        | P2.8         |
      +---------------+--------------+
      | ANT_SW        | P2.10        |
      +---------------+--------------+
      | NSS           | P0.0         |
      +---------------+--------------+
      | SPI MOSI      | P0.1         |
      +---------------+--------------+
      | SPI MISO      | P0.2         |
      +---------------+--------------+
      | SPI SCK       | P0.3         |
      +---------------+--------------+

.. _setting_up_sidewalk_gateway:

Sidewalk gateway
****************

.. note::
   Refer to the `Amazon Sidewalk gateways`_ for the complete list of available devices.

See the official Amazon Sidewalk documentation on `Setting up a Sidewalk gateway`_.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

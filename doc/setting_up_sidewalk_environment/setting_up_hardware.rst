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

   .. tab:: nRF54L15 PDK

      The nRF54L15 PDK does not have the Arduino-compatible header.
      The connection layout may differ depending on the revision of your PDK.
      GPIO assignments are recommended by the Nordic Semiconductor.

      The nRF54L15 PDK operates on a lower voltage level (1.8V).
      The Semtech Shield is compatible with this voltage and can be safely provided to the pin marked as 3.3V.
      Due to the lower voltage, the Semtech radio cannot achieve the full transmit power, and it will reach a plateau at 15 dBm gain even when a higher value is configured.
      The specification of the Semtech module describes that it is possible to provide a separate, higher voltage source for radio while maintaining the lower voltage for communication.
      However, the recommended shield does not support this configuration.

      .. tabs::

         .. tab:: PDK revision v0.2.1

            The v0.2.1 PDK revision does not require any hardware modification, but the buttons are reused to connect the Semtech SX126X shield.
            Pressing the buttons while working with the Semtech module can lead to malfunctions and undefined behavior.
            This issue is caused by the limitation of the v0.2.1 PDK revision.
            It is possible to assign different pins for the radio shield, but you might need to disable some of the PDK's functions, or modify connections by cutting or soldering PDK bridges.

            +---------------+--------------+
            | SX126X shield | nRF54L15 PDK |
            +===============+==============+
            | BUSY          | P1.12        |
            +---------------+--------------+
            | DIO1          | P1.10        |
            +---------------+--------------+
            | NSS           | P2.10        |
            +---------------+--------------+
            | ANT_SW        | P2.07        |
            +---------------+--------------+
            | SPI MOSI      | P2.08        |
            +---------------+--------------+
            | SPI MISO      | P2.09        |
            +---------------+--------------+
            | SPI SCK       | P2.06        |
            +---------------+--------------+
            | NRESET        | P1.11        |
            +---------------+--------------+

         .. tab:: PDK revision v0.3.0 / v0.7.0

            To use the suggested pins, disable **UART0** (not used by Sidewalk samples) through the `Board Configurator`_ tool in the nRF Connect for Desktop.
            This step is required for the shield to work as some pins are connected to **UART0** by default.

            See a board configuration for the suggested pin layout:

            +--------------------------------------------------------------+
            |                                                              |
            | .. figure:: /images/board_configurator_nrf54l15_0_3_0.png    |
            |                                                              |
            +--------------------------------------------------------------+

            +---------------+--------------+
            | SX126X shield | nRF54L15 PDK |
            +===============+==============+
            | BUSY          | P0.00        |
            +---------------+--------------+
            | DIO1          | P0.03        |
            +---------------+--------------+
            | NSS           | P2.10        |
            +---------------+--------------+
            | ANT_SW        | P0.01        |
            +---------------+--------------+
            | SPI MOSI      | P2.08        |
            +---------------+--------------+
            | SPI MISO      | P1.11        |
            +---------------+--------------+
            | SPI SCK       | P2.06        |
            +---------------+--------------+
            | NRESET        | P0.02        |
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

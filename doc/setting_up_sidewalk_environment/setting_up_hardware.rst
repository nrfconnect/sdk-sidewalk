.. _setting_up_hardware:

Setting up hardware
###################

Before setting up the Sidewalk environment, you need to make sure you have correctly set up the required hardware.

Development kit
***************

The nRF52840 DK is required to run the Sidewalk samples.
For information on how to start working with the device, refer to `Getting started with nRF52 Series`_.

Semtech SX1262
**************

Sidewalk samples use sub-GHz radio transceivers to support LoRa and (G)FSK modulation.

For evaluation, use the `Semtech SX1262MB2CAS`_ Arduino Shield.

.. figure:: /images/nRF52840-Semtech-SX1262.png

.. _setting_up_hardware_semtech_pinout:

Pinout
------

Connect the Semtech shield to the Arduino-compatible headers on the nRF52840 DK.
Refer to the pinout assignment:

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


.. _Getting started with nRF52 Series: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/working_with_nrf/nrf52/gs.html
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

.. _setting_up_hardware:

Setting up hardware
###################

Before setting up the Sidewalk environment, you need to make sure you have correctly set up the required hardware.

Development Kit
***************

For information on how to start working with the nRF52840 DK, refer to the `Get started`_ page.

Semtech SX1262
**************

Sidewalk samples use sub-GHz radio transceivers to support LoRa and (G)FSK modulation.

For evaluation, use the `Semtech SX1262MB2CAS`_ Arduino Shield.
Connect the Semtech shield to the Arduino-compatible headers on the nRF52840 DK.

.. figure:: /images/nRF52840-Semtech-SX1262.png

Pinout
******

+---------------+----------+
| SX126X shield | nRF52840 |
+===============+==========+
| BUSY          | P1.04    |
+---------------+----------+
| DIO1          | P1.06    |
+---------------+----------+
| NSS           | P1.08    |
+---------------+----------+
| ANT_SW        | P1.10    |
+---------------+----------+
| SPI MOSI      | P1.13    |
+---------------+----------+
| SPI MISO      | P1.14    |
+---------------+----------+
| SPI SCK       | P1.15    |
+---------------+----------+
| NRESET        | P0.03    |
+---------------+----------+


.. _Get started: https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK/GetStarted?lang=en#infotabs
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

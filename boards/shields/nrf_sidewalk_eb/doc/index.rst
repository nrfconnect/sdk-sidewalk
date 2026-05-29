.. nrf_sidewalk_eb:

nRF Sidewalk EB shield
######################

Overview
********

The nRF Sidewalk EB shield is a Nordic Semiconductor expansion board for the
nRF54L15 DK and nRF54LM20 DK. It mounts on the DK expansion port and carries
an LR1110 radio module (LBAA0XV2DT) with antennas for LoRa, Wi-Fi, and GNSS,
plus a status LED.

Supported boards
================

* ``nrf54l15dk/nrf54l15/cpuapp``
* ``nrf54l15dk/nrf54l15/cpuapp/ns``
* ``nrf54l15dk/nrf54l10/cpuapp``
* ``nrf54lm20dk/nrf54lm20a/cpuapp``
* ``nrf54lm20dk/nrf54lm20b/cpuapp``

Pin assignment
==============

Control signals use the ``nordic_expansion_header`` pin indices from the shield
devicetree. SPI uses the ``nordic_expansion_spi`` bus on the DK.

nRF54L15 DK
-----------

+------------------+---------------------+------------------+
| Expansion pin    | Function            | SoC pin          |
+==================+=====================+==================+
| 6                | LR1110 SPI MOSI     | P1.06            |
+------------------+---------------------+------------------+
| 7                | LR1110 SPI MISO     | P1.07            |
+------------------+---------------------+------------------+
| 8                | LED                 | P1.08            |
+------------------+---------------------+------------------+
| 9                | LR1110 GNSS LNA     | P1.09            |
+------------------+---------------------+------------------+
| 10               | LR1110 SPI NSS      | P1.10            |
+------------------+---------------------+------------------+
| 11               | LR1110 SPI SCK      | P1.11            |
+------------------+---------------------+------------------+
| 12               | LR1110 NRESET       | P1.12            |
+------------------+---------------------+------------------+
| 13               | LR1110 EVENT        | P1.13            |
+------------------+---------------------+------------------+
| 14               | LR1110 BUSY         | P1.14            |
+------------------+---------------------+------------------+

nRF54LM20 DK
------------

+------------------+---------------------+------------------+
| Expansion pin    | Function            | SoC pin          |
+==================+=====================+==================+
| 6                | LR1110 SPI MOSI     | P3.00            |
+------------------+---------------------+------------------+
| 7                | LR1110 SPI MISO     | P3.01            |
+------------------+---------------------+------------------+
| 8                | LED                 | P0.03            |
+------------------+---------------------+------------------+
| 9                | LR1110 GNSS LNA     | P0.04            |
+------------------+---------------------+------------------+
| 10               | LR1110 SPI NSS      | P3.02            |
+------------------+---------------------+------------------+
| 11               | LR1110 SPI SCK      | P3.03            |
+------------------+---------------------+------------------+
| 12               | LR1110 NRESET       | P1.07            |
+------------------+---------------------+------------------+
| 13               | LR1110 EVENT        | P1.06            |
+------------------+---------------------+------------------+
| 14               | LR1110 BUSY         | P1.05            |
+------------------+---------------------+------------------+

Requirements
************

nRF54L15 DK
===========

The shield connects to the **Port P1** expansion header. Several on-board DK
resources share pins with the LR1110:

* Do not press **Button 0** (P1.13), **Button 1** (P1.09), or **Button 2** (P1.08)
  while the shield is connected. These pins are used for LR1110 EVENT, GNSS LNA,
  and the shield LED, respectively.
* **LED 1** (P1.10) and **LED 3** (P1.14) share pins with LR1110 SPI NSS and BUSY
  and are unavailable while the shield is connected.
* Do not enable **UART20** hardware flow control. RTS and CTS are routed to
  P1.06 and P1.07, which are used for SPI MOSI and MISO.

Programming
***********

Set ``-DSHIELD=nrf_sidewalk_eb`` when you invoke ``west build``. For example:

.. zephyr-app-commands::
   :zephyr-app: samples/subsys/lorawan/class_a
   :board: nrf54lm20dk/nrf54lm20b/cpuapp
   :shield: nrf_sidewalk_eb
   :goals: build

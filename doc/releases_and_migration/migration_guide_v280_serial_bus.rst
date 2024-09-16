.. _migration_guide_v280_serial_bus:

Sidewalk PAL serial bus for nRF52840 SoC
****************************************

Reasons for change
==================

Due to fact that Zephyr's SPI driver adds additional delay to the SPI bus communication, which have significant impact to the
FSK protocol for nRF52840, hence the new implementation of the PAL serial bus is introduced.

Aligning your application to the new PAL serial bus
===================================================

The new PAL serial bus implementation includes the following changes:

* Added the :file:`sid_pal_serial_bus_nrfx_spi.c` file with the PAL SPI bus implementation based on the NRFX SPI driver.
  This PAL is used only for nRF52840 SoC build and it has no impact to other platforms.

* The new ``CONFIG_SIDEWALK_NRFX_SPI_INSTANCE_ID`` Kconfig option for selecting SPI bus instance for communication with an external
  sub-GHz transceiver.

* Added the ``nrfx_spi_gpios`` child node in the :file:`nrf52840dk_nrf52840.overlay` overlay for configuring the
  SPI bus ``CLK``, ``MOSI``, ``MISO`` and ``CS`` pins.

  .. note::
    When SPI bus instance is selected by ``CONFIG_SIDEWALK_NRFX_SPI_INSTANCE_ID`` Kconfig option, it is strongly recommended
    to disable it in the devicetree.

.. _migration_guide_addon_v110:

Migration guide for moving to Sidewalk Add-on v1.1.0
####################################################

Overview
========

This migration guide covers the transition from Sidewalk Add-on v1.0.1 to v1.1.0.
The main changes include:

- :ref:`Semtech radio <semtech_radio_migration>` - Add new radio, refactor the radio configuration (see ).

.. Important: Existing SX126x users can continue without changes,
   but new features require migration to the shield system.

Breaking Changes
================

.. Important: These changes require code modifications

- **API Change**: ``get_radio_cfg()`` function signature changed from ``const radio_sx126x_device_config_t*`` to ``const void*``
- **File Deletion**: :file:`subsys/config/common/src/app_subGHz_config.c` removed and replaced with radio-specific files
- **Build System**: Radio drivers now use conditional compilation based on devicetree

.. _semtech_radio_migration:

Semtech radio
=============

Starting from this release, the direct board overlay configuration for Semtech SX126x radio only has been replaced with standardized DTS bindings for Semtech SX126x and LR1110 radios.

Key changes:
- **Architecture**: From single-radio to configurable radio architecture
- **Devicetree Sources**: From basic GPIO and SPI labels to advanced radio description
- **Samples**: From board-specific to shield-based configuration

Architecture
************

SubGHz radio configuration in Sidewalk is based on `Zephyr Devicetree Sources <https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/topic/devicetree>`_ with hardware description. It is a common approach for all nRF Connect SDK applications.

Starting from this release, Semtech radio devicetree bindings (YAML files) are based on `LoRa Basics Modem integration in Zephyr OS <https://github.com/Lora-net/LBM_Zephyr>`_. This standardizes radio description and allows faster transition from LoRaWAN to Sidewalk projects based on Zephyr OS.

Sidewalk Platform Abstraction Layer for subGHz radio uses Sidewalk-specific implementation (different from LoRa Basic Modem in Zephyr). Sidewalk drivers do not follow the Zephyr Device Driver model. Instead, hardware configuration from devicetree sources is translated to Sidewalk driver configuration structures in app_subGHz_config files.

Semtech radio parameters configured in devicetree:

- GPIOs
- SPI
- TCXO config
- Regulator mode
- RX boost
- RF switch (LR1110 only)

Semtech radio parameters configured only in app_subGHz_config file:

- LNA gain
- State timings
- Regional config

File changes
************

The following files were impacted by the new radio approach:

- **app_subGHz_config**: :file:`subsys/config/common/src/app_subGHz_config.c` was deleted and replaced with radio-specific files: :file:`app_subGHz_config_lr11xx.c` and :file:`app_subGHz_config_sx126x.c`.
- **get_radio_cfg**: ``const radio_sx126x_device_config_t* get_radio_cfg(void)`` was replaced with ``const void* get_radio_cfg(void);``. This affects sidewalk platform initialization in :file:`samples/sid_end_device/src/sidewalk_events.c` and :file:`subsys/sal/sid_pal/src/sid_common.c`.
- **Devicetree**: Added radio hardware description in: :file:`dts/bindings/lora_lbm` and :file:`include/zephyr/dt-bindings/lora_lbm`.
- **Shields**: Added Arduino-compatible Semtech radio shields in :file:`boards/shields`.

New radio LR1110
****************

Support for the new Semtech LR1110 radio was added. The major changes included:

- DTS bindings: :file:`dts/bindings/lora_lbm/semtech,lr1110.yaml`
- Driver: :file:`subsys/semtech/lr1110`
- Shield: :file:`boards/shields/semtech_lr11xxmb1xxs/`
- Radio config: :file:`subsys/config/common/src/app_subGHz_config_lr11xx.c`

Samples
*******

Sidewalk transport protocol configuration (BLE, FSK, LoRa) is chosen automatically based on hardware description. Use Zephyr `shield <https://docs.nordicsemi.com/bundle/ncs-3.0.0/page/zephyr/hardware/porting/shields.html#shield_activation>`_ to describe your hardware (nRF development boards and attached modules).

.. tabs::
    .. tab:: New Shield-based configuration

            BLE only

            .. code-block:: console

               west build -b nrf52840dk/nrf52840

            Semtech SX126x radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 --shield semtech_sx126xmb2xxs

            Semtech LR1110 radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 --shield semtech_lr11xxmb1xxs

            Semtech radio on nRF54L15 DK

            .. code-block:: console

               west build -b nrf54l15dk/nrf54l15/cpuapp --shield simple_arduino_adapter --shield semtech_sx126xmb2xxs

    .. tab:: Old board-specific configuration

            BLE only

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 -- -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

            Semtech SX126x radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840

            Semtech LR1110 radio

                Not supported

            Semtech radio on nRF54L15 DK

            .. code-block:: console

               west build -b nrf54l15dk/nrf54l15/cpuapp

Devicetree Sources
******************

Example DTS code snippets for nRF54L15 and SX126x hardware are shown below.

.. tabs::

    .. tab:: New radio description

        .. code-block:: dts

            #include <zephyr/dt-bindings/lora_lbm/sx126x.h>

            / {
                chosen {
                    zephyr,lora-transceiver = &lora_semtech_sx126xmb2xxs;
                };
                aliases {
                    lora-transceiver = &lora_semtech_sx126xmb2xxs;
                };
            };

            &spi30 {
                status = "okay";
                pinctrl-0 = <&spi30_default_alt>;
                pinctrl-1 = <&spi30_sleep_alt>;
                pinctrl-names = "default", "sleep";
                cs-gpios = <&gpio0 0 GPIO_ACTIVE_LOW>;
                lora_semtech_sx126xmb2xxs: lora@0 {
                    reg = <0>;
                    spi-max-frequency = <DT_FREQ_M(8)>;
                    reset-gpios = <&gpio2 8 GPIO_ACTIVE_LOW>;
                    busy-gpios = <&gpio2 6 GPIO_ACTIVE_HIGH>;
                    dio1-gpios = <&gpio1 11 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>;
                    dio2-as-rf-switch;
                    reg-mode = <SX126X_REG_MODE_LDO>;
                    tcxo-wakeup-time = <0>;
                    tcxo-voltage = <SX126X_TCXO_SUPPLY_1_8V>;
                };
            };

            &pinctrl {
                spi30_default_alt: spi30_default_alt {
                    group1 {
                        psels = <NRF_PSEL(SPIM_SCK, 0, 3)>,
                                <NRF_PSEL(SPIM_MISO, 0, 2)>,
                                <NRF_PSEL(SPIM_MOSI, 0, 1)>;
                    };
                };

                spi30_sleep_alt: spi30_sleep_alt {
                    group1 {
                        psels = <NRF_PSEL(SPIM_SCK, 0, 3)>,
                                <NRF_PSEL(SPIM_MISO, 0, 2)>,
                                <NRF_PSEL(SPIM_MOSI, 0, 1)>;
                        low-power-enable;
                    };
                };
            };

        Notes:

        - Compatible with :file:`dts/bindings/lora_lbm`
        - Semtech radio ``chosen`` as zephyr lora transceiver
        - Semtech radio is a sub-node for SPI node

    .. tab:: Old GPIO and SPI labels

        .. code-block:: dts

            /{
                semtech_sx1262_gpios{
                    compatible = "gpio-keys";
                    semtech_sx1262_reset_gpios: reset {
                        gpios = <&gpio2 8  (GPIO_ACTIVE_LOW|GPIO_PULL_UP)>;
                        label = "semtech_sx1262 Reset";
                    };
                    semtech_sx1262_busy_gpios: busy {
                        gpios = <&gpio2 6 (GPIO_ACTIVE_HIGH)>;
                        label = "semtech_sx1262 Busy";
                    };
                    semtech_sx1262_antenna_enable_gpios: antena_enable {
                        gpios = <&gpio2 10 (GPIO_ACTIVE_HIGH)>;
                        label = "semtech_sx1262 Antena Enable";
                    };
                    semtech_sx1262_dio1_gpios: dio1 {
                        gpios = <&gpio1 11  (GPIO_ACTIVE_HIGH|GPIO_PULL_DOWN)>;
                        label = "semtech_sx1262 DIO1";
                    };
                };
            };

            sid_semtech: &spi30 {
                compatible = "nordic,nrf-spim";
                status = "okay";
                cs-gpios = <&gpio0 0 GPIO_ACTIVE_LOW>;
                pinctrl-0 = <&spi30_default_alt>;
                pinctrl-1 = <&spi30_sleep_alt>;
                pinctrl-names = "default", "sleep";
                clock-frequency = <DT_FREQ_M(8)>;
            };

            &pinctrl {
                spi30_default_alt: spi30_default_alt {
                    group1 {
                        psels = <NRF_PSEL(SPIM_SCK, 0, 3)>,
                            <NRF_PSEL(SPIM_MISO, 0, 2)>,
                            <NRF_PSEL(SPIM_MOSI, 0, 1)>;
                    };
                };

                spi30_sleep_alt: spi30_sleep_alt {
                    group1 {
                        psels = <NRF_PSEL(SPIM_SCK, 0, 3)>,
                            <NRF_PSEL(SPIM_MISO, 0, 2)>,
                            <NRF_PSEL(SPIM_MOSI, 0, 1)>;
                        low-power-enable;
                    };
                };
            };

        Notes:

        - SPI instance alias ``sid_semtech``
        - Semtech GPIOs defined separately

SPI driver from nrfx instead of Zephyr
**************************************

The ``spi_nrfx`` `snippet <https://docs.nordicsemi.com/bundle/ncs-3.0.0/page/zephyr/build/snippets/using.html>`_ in Sidewalk sample adds necessary changes in Kconfigs and DTS to use nrfx SPI driver. The nrfx SPI driver is recommended on nRF52 designs with FSK transport protocol.

.. Important: It is important to add the snippet to the ``sid_end_device`` namespace only.
   Otherwise, build issues in mcuboot image build will fail.

.. code-block:: console

    west build -b nrf52840dk/nrf52840 -p -- -DSHIELD="semtech_sx126xmb2xxs" -Dsid_end_device_SNIPPET="spi_nrfx"


Troubleshooting
===============

Common issues and solutions:

.. TODO: Troubleshooting:
.. no sidewalk in west (no boards, dts, sources)
.. no lora-transceiver in zephyr, no spi
.. etc

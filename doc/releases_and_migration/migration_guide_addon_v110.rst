.. _migration_guide_addon_v110:

Migration guide for moving to Sidewalk Add-on v1.1.0
####################################################


Semtech radio
=============

Starting from this release, the direct board overlay configuration for Semtech SX126x radio only was replaced with standardized DTS bindings for Semtech SX126x and LR1110 radios.

Key changes:
- **Architecture**: from single-radio to multi-radio architecture
- **Devicetree Sources**: from basic GPIO and SPI label to advanced radio description
- **Samples**: from board-specific to shield-based configuration

Architecture
************

SubGHz radio configuration in Sidewalk is based on `Zephyr Devicetree Sources <https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/topic/devicetree>`_ with a hardware description. It a common approach for all nRF Connect SDK applications.

Starting from this release, Semtech radios devicetree bindings (YAML files) are based on `LoRa Basics Modem integration in Zephyr OS <https://github.com/Lora-net/LBM_Zephyr>`_. It standardize radio description and allows to faster transition form LoRaWAN to Sidewalk projects based on Zephyr OS.

Sidewalk Platform Abstraction Layer for subGHz radio uses Sidewalk specific implementation (different than Lora Basic Modem in Zephyr). Sidewalk drivers doesn't follow Zephyr Device Driver model, instead hardware configuration form devicetree sources is translated to Sidewalk driver configuration structures in app_subGHz_config files.

Semtech radio parameters configured in devicetree:
- GPIOs
- SPI
- TCXO config
- regulator mode
- rx boost
- rfswitch (lr110 only)

Semtech radio parameters configured only in app_subGHz_config file:
- lna gain
- state timings
- regional config

File changes
************

The following files was impacted by new radio approach:
- app_subGHz_config: ``subsys/config/common/src/app_subGHz_config.c ``was deleted and replaced with radio specyfic files: ``app_subGHz_config_lr11xx.c``  and ``app_subGHz_config_sx126x.c``.
- get_radio_cfg: ``const radio_sx126x_device_config_t* get_radio_cfg(void)`` was replaced with ``const void* get_radio_cfg(void);``. It affects sidewalk platform initialization in ``sidewalk_event_platform_init`` (from ``samples/sid_end_device/src/sidewalk_events.c``) and ``sid_pal_common_init``(from ``subsys/sal/sid_pal/src/sid_common.c``)
- Devicetree: Added radio hardware description in: ``dts/bindings/lora_lbm`` and ``include/zephyr/dt-bindings/lora_lbm``.
- Shields: Added Arduino compatible Semtech radio shields in ``sidewalk/boards/shields``

New radio lr1110
****************

The support for new Semtech radio was added. The major changes included:

- DTS bindings dts/bindings/lora_lbm/semtech,lr1110.yaml
- Driver subsys/semtech/lr1110
- Shield boards/shields/semtech_lr11xxmb1xxs/
- Radio config ubsys/config/common/src/app_subGHz_config_lr11xx.c

Samples
*******

Sidewalk transport protocol configuration (BLE, FSK, LoRa) is chosen automatically based on hardware description. Use Zephyr `shield <https://docs.nordicsemi.com/bundle/ncs-3.0.0/page/zephyr/hardware/porting/shields.html#shield_activation>`_ to describe your hardware (nRF development boards and attached modules).

.. tabs::
    .. tab:: New Shield-based configuration

            BLE only

            .. code-block:: console

               west build -b nrf52840dk/nrf52840

            Semtech sx1262 radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 --shield semtech_sx1262mb2cas

            Semtech lr1110 radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 --shield semtech_lr1110mb1kas

            Semtech radio on nRF54L15 DK

            .. code-block:: console

               west build -b nrf54l15dk/nrf54l15/cpuapp --shield simple_arduino_adapter --shield semtech_sx1262mb2cas

    .. tab:: Old board-specific configuration

            BLE only

            .. code-block:: console

               west build -b nrf52840dk/nrf52840 -- -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

            Semtech sx1262 radio

            .. code-block:: console

               west build -b nrf52840dk/nrf52840

            Semtech lr1110 radio

                Not supported

            Semtech radio on nRF54L15 DK

            .. code-block:: console

               west build -b nrf54l15dk/nrf54l15/cpuapp

Devicetree Sources
******************

Example dts code snippet for nRF54l15 and sx126x hardware are shown below.

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
                    dio1-gpios = <&&gpio1 11 (GPIO_ACTIVE_HIGH | GPIO_PULL_DOWN)>;
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

        - Compatible with ``sidewalk/dts/bindings/lora_lbm``
        - Semtech radio ``chosen`` as zephyr lora transceiver
        - Semtech radio is a sub-node for spi node

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
        - Semtech gpios defined separately

SPI driver from nrfx instead of Zephyr
**************************************

The ``spi_nrfx`` `snippet <https://docs.nordicsemi.com/bundle/ncs-3.0.0/page/zephyr/build/snippets/using.html>`_ in Sidewalk sample added necessary changes in kconfigs and dts to use nrfx spi driver. The nrfx spi driver is recommended on nRF52 design with FSK transport protocol.
It's important to add snippet to ``sid_end_device`` namespace only. In other case build issues in mcuboot image build fails.

.. code-block:: console

    west build -b nrf52840dk/nrf52840 -p -- -DSHIELD="semtech_sx1262mb2cas" -Dsid_end_device_SNIPPET="spi_nrfx"

.. TODO: Troubleshooting:
.. no sidewalk in west (no boards, dts, sources)
.. no lora-transceiver in zephyr, no spi
.. etc

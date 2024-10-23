.. _migration_guide_v280:

Migration guide for the v2.8.0 of the nRF Connect SDK
#####################################################

.. contents::
   :local:
   :depth: 2

Starting with the v2.8.0 release of the `nRF Connect SDK`_, there is a new Sidewalk provisioning module, PAL serial bus for the nRF52840 SoC and new Sidewalk libraries v1.17.
For an overview of changes, read the following sections:

.. _migration_guide_v280_prov_module:

Sidewalk provisioning module
****************************

The new provisioning module enhances security by:

* Moving non-volatile Sidewalk keys to secure key storage.
* Protecting the manufacturing data partition from write operations.
* Always using the Key-Value Store version of manufacturing data.

Security enhancements details
=============================

Access to the manufacturing data is unified within the application, ensuring consistency for prototyping and manufacturing flows.
The MFG parser module is launched at initialization and writes the manufacturing data to flash (secure key storage) in the same way as the device certification module.
The process is triggered only once, at the first application start after flashing the :file:`Nordic_MFG.hex` file.
The following logs are generated:

.. code-block:: console

   [00:00:00.004,502] <inf> sid_mfg: Need to parse mfg data
   [00:00:00.009,993] <inf> sid_mfg_parser_v8: MFG_ED25519 import success
   [00:00:00.014,900] <inf> sid_mfg_parser_v8: MFG_SECP_256R1 import success
   [00:00:00.054,505] <inf> sid_mfg: Successfully parsed mfg data

The secure key storage is based on the `Trusted storage`_ security library.
After keys are imported into secure key storage, it is impossible to read their raw values.
Therefore, once secure key storage is enabled in the firmware, it must remain enabled in all subsequent firmware versions.
This feature is enabled by default in all Sidewalk samples.

The hardware flash write protection is based on the `Hardware flash write protection`_ security library.
The protection is applied during Sidewalk initialization and remains irreversible until reset.
This feature is disabled in the :ref:`variant_sidewalk_dut` sample to allow writing of manufacturing data using on-device certification commands.

Aligning your application to the new provisioning module
========================================================

The new provisioning module implementation includes the following changes:

* In the :file:`app_mfg_config.h` file, the function ``app_mfg_cfg_is_valid()`` was renamed to ``app_mfg_cfg_is_empty()``.
* The ``CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7`` Kconfig option adds support for older Sidewalk manufacturing HEX formats (version 7 and below).
  You can disable this configuration when using the latest manufacturing flash.

Using old provisioning module
=============================

It is recommended to use the new provisioning module implementation.
However, you can still enable the old module using the ``CONFIG_DEPRECATED_SIDEWALK_MFG_STORAGE`` Kconfig option.

.. _migration_guide_v280_serial_bus:

Sidewalk PAL serial bus for the nRF52840 SoC
********************************************

The new implementation of the PAL serial bus has been introduced to address additional delays caused by Zephyr's SPI driver in SPI bus communication.
These delays significantly impact the FSK protocol for the nRF52840 SoC.

Aligning your application to the new PAL serial bus
===================================================

The new PAL serial bus implementation includes the following changes:

* Added the :file:`sid_pal_serial_bus_nrfx_spi.c` file with the PAL SPI bus implementation based on the NRFX SPI driver.
  This PAL is used only for the nRF52840 SoC build and does not affect other platforms.
* Added the new ``CONFIG_SIDEWALK_NRFX_SPI_INSTANCE_ID`` Kconfig option, allowing to select the SPI bus instance for communication with an external sub-GHz transceiver.
* Added the ``nrfx_spi_gpios`` child node in the :file:`nrf52840dk_nrf52840.overlay` overlay for configuring the SPI bus ``CLK``, ``MOSI``, ``MISO``, and ``CS`` pins.

  .. note::
    It is recommended to disable the SPI bus instance in the devicetree when it is selected by the ``CONFIG_SIDEWALK_NRFX_SPI_INSTANCE_ID`` Kconfig option.

.. _migration_guide_v280_new_libraries:

Sidewalk v1.17 libraries
************************

The version 1.17 of Sidewalk libraries introduces metrics and capability features, along with fixes for known issues found in previous versions.

New end device characteristics
==============================

A new characteristic has been added to the :ref:`sidewalk_end_device` configuration. 
This characteristic includes device type, power source, and qualification ID. 
To add this information, you must populate the ``dev_ch`` fields in the ``sid_config`` structure before the Sidewalk stack starts.

See the following example of device characteristics:

.. code-block:: C

	struct sid_end_device_characteristics dev_ch = {
		.type = SID_END_DEVICE_TYPE_STATIC,
		.power_type = SID_END_DEVICE_POWERED_BY_BATTERY_AND_LINE_POWER,
		.qualification_id = 0x0001,
	};

	sid_ctx.config = (struct sid_config){
		.dev_ch = dev_ch,
		.callbacks = &event_callbacks,
		.link_config = app_get_ble_config(),
		.sub_ghz_link_config = app_get_sub_ghz_config(),
  };


New Sidewalk ID option
======================

A new option has been introduced that allows fetching of the Sidewalk ID for the device. 
This option is functional only after the device has been registered, as the Sidewalk ID is assigned post-registration.

The Sidewalk ID can be retrieved using the ``-gsi`` subcommand of the CLI (DUT) option:

.. code-block:: console

  uart:~$ sid last_status 
  [00:00:51.950,347] <inf> sid_cli: Device Is registered, Time Sync Fail, Link status: {BLE: Down, FSK: Down, LoRa: Down}
  uart:~$ sid option -gsi
  [00:00:55.582,641] <inf> sid_cli: sid_option returned 0 (SID_ERROR_NONE); SIDEWALK_ID: BFFFFFABCD

Fixing uplink buffer
====================

Upon migration, you can remove the ``pending_message_list`` workaround, as the first uplink message after Link Up payload has been fixed in the v2.8.0 release. 
For the issue details, see Amazon Sidewalk Known Issues.

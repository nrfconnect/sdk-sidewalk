.. _migration_guide_addon_v200:

Migration guide for Amazon Sidewalk Add-On v2.0.0
#################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.3.0 to v2.0.0.

Overview
********

Amazon Sidewalk Add-On v2.0.0 upgrades the nRF Connect SDK to v3.4.1.
Refer to the :ref:`sidewalk_release_notes_addon_v200` for more details.

Migration steps
***************

The following sections describe the changes that might require an update of your application.

.. _migration_200_region:

Sub-GHz region selection
========================

The Sidewalk v2.0 libraries cannot initialize the LoRa and FSK links until the regulatory region of the device is set.
The add-on sets the region during the platform initialization, based on the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice.
The default ``CONFIG_SIDEWALK_SUBGHZ_REGION_US915`` option keeps the behavior of the previous add-on releases.
Select ``CONFIG_SIDEWALK_SUBGHZ_REGION_EU868`` for devices operating in Europe.

If your application calls ``sid_platform_init()`` directly instead of using the sample code, set the following fields of ``platform_parameters_t.platform_init_parameters``:

* ``region`` - The region code, ``SID_PAL_RADIO_RC_NA`` or ``SID_PAL_RADIO_RC_EU``.
  If you leave it as ``SID_PAL_RADIO_RC_NONE``, the LoRa and FSK links are not available.
* ``sub_ghz_cfg`` - The sub-GHz link configuration passed to ``sid_init()``.
  The platform initialization updates its maximum transmit power for the selected region.
* ``iso_country`` - Optional ISO 3166-1 alpha-2 country code, or ``NULL``.

See :ref:`sidewalk_configuration` for more information.

.. _migration_200_libraries:

Prebuilt library layout
=======================

The prebuilt Sidewalk libraries moved from the :file:`lib/cortex-m*` directories to the :file:`lib/release` and :file:`lib/debug` directories.
The release variant is used by default.
If your build system refers to the library paths directly, update it, or use the ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` and ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig options instead.

The Sidewalk utility headers, such as :file:`sid_utils.h`, moved from :file:`subsys/app_utils/config/include` to the :file:`subsys/sal/common/sid_utility_ifc` directory, which is provided by the Sidewalk SDK.
The include path is added by the add-on build system, so you only need an update if you refer to the old directory directly.

.. _migration_200_api:

Sidewalk API changes
====================

The following Sidewalk API changes most often require an update of the application code.
For the complete list, including the platform abstraction layer changes, see the `Amazon Sidewalk MCU SDK Migration Guide`_.

* Location Library - The API changes are not backward compatible:

  * ``SID_LOCATION_METHOD_BLE_GATEWAY`` is renamed to ``SID_LOCATION_METHOD_GATEWAY``.
  * The ``buffer`` and ``size`` fields of ``sid_location_run_config`` are replaced with the typed ``payload_type`` and ``payload`` fields.
  * The ``payload``, ``size`` and ``raw_gnss_data`` fields of ``sid_location_result`` are replaced with the typed ``result_type`` and ``parsed`` fields.
    Use the ``sid_location_parse_wifi()`` and ``sid_location_parse_gnss()`` functions to decode them.
  * The ``sid_location_update_almanac()`` function is removed.
  * The ``SID_LOCATION_EFFORT_INVALID`` and ``SID_LOCATION_EFFORT_DEFAULT`` values change, because new location levels are added.
    Do not store or compare these values as numbers.

  For an example of the new API, see the location shell of the :ref:`variant_sidewalk_dut` variant in the :file:`samples/sid_end_device/src/cli/location_shell_events.c` file.

* ``sid_msg_desc_tx_attributes.resolve_location`` is now a bit mask.
  Replace ``true`` with ``SID_TX_LOCATION_LINK_RESOLVE``.

* ``struct sid_config`` has a new ``gsk_config`` field for the Gateway Session Key (GSK) configuration.
  GSK is enabled by default in the Sidewalk v2.0 libraries.
  Initialize the whole structure, for example with a designated initializer, so that ``gsk_config`` is ``NULL`` and the default GSK configuration is used.
  The :ref:`sidewalk_end_device` sample already does this.

* ``struct sid_sub_ghz_links_config`` and ``struct sid_device_profile_unicast_params`` have new trailing fields.
  If you initialize them with positional initializers, change them to designated initializers.

* ``sid_dlrv_config.nwsync_time`` is now a 16-bit value.
  Check the code that stores or compares it as an 8-bit value.

* ``SID_BULK_DATA_TRANSFER_FILE_DESCRIPTOR_MAX_SIZE`` grows from 128 to 200 bytes.
  Check the buffers sized with this constant.

* Manufacturing store value identifiers 4001 to 4008 are now reserved by the Sidewalk SDK.
  If your application stores its own values under these identifiers, move them.

.. _migration_200_kconfig:

Renamed Kconfig option
======================

The ``SIDEWALK_MFG_STORAGE_DIAGNOSTIC`` Kconfig option is renamed to ``SIDEWALK_MFG_ALLOW_WRITE``.
The option is still enabled automatically by ``CONFIG_SIDEWALK_ON_DEV_CERT``.
If your code checks ``CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC``, use ``CONFIG_SIDEWALK_MFG_ALLOW_WRITE`` instead.

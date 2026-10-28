.. _migration_guide_addon_v200:

Migration guide for Amazon Sidewalk Add-On v2.0.0
#################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.3.0 to v2.0.0.

Overview
********

Amazon Sidewalk Add-On v2.0.0 upgrades the nRF Connect SDK to v3.4.1 and the Amazon Sidewalk libraries to the Sidewalk SDK v2.0 release.
This release adds Europe region support, integrates the v2.0 sub-GHz platform abstraction layer, and updates the :ref:`sidewalk_end_device` sample for the new Location Library API.
The Sidewalk SDK v2.0 breaks backward compatibility for the Location Library and related messaging APIs.

Refer to the :ref:`sidewalk_release_notes_addon_v200` for more details.

Migration steps
***************

#. Navigate to the Amazon Sidewalk Add-On repository (the :file:`sidewalk` directory in your west workspace).

#. Update the Add-On repository to the release tag ``v2.0.0-add-on`` from the `sdk-sidewalk`_ GitHub repository.

   The commands below assume a Git remote named ``ncs`` that points to `sdk-sidewalk`_.
   If your remote uses a different name, substitute it for ``ncs``.

   .. code-block:: console

      git fetch ncs
      git checkout v2.0.0-add-on

#. From your west workspace root (the parent directory of :file:`sidewalk`), update the nRF Connect SDK and other west manifest projects:

   .. code-block:: console

      west update

#. Perform a clean build:

   .. code-block:: console

      west build -p -b <board_target> <sidewalk_application>

After a successful build, review the following sections if you use sub-GHz links, custom library paths, or the location Library.

.. _migration_200_region:

Sub-GHz region selection
========================

The v2.0 libraries cannot initialize LoRa and FSK links until you select the device regulatory region.
The add-on sets the region during platform initialization from the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice.
Keep the default ``CONFIG_SIDEWALK_SUBGHZ_REGION_US915`` Kconfig option for devices that operate in North America.
For devices that operate in Europe, select the ``CONFIG_SIDEWALK_SUBGHZ_REGION_EU868`` Kconfig option.

If your application calls the ``sid_platform_init()`` function directly instead of using the sample code, set the following fields in the ``platform_parameters_t.platform_init_parameters`` structure:

* ``region`` — Region code ``SID_PAL_RADIO_RC_NA`` (North America) or ``SID_PAL_RADIO_RC_EU`` (Europe).
  If you leave the ``region`` field as ``SID_PAL_RADIO_RC_NONE``, LoRa and FSK links are unavailable.
* ``sub_ghz_cfg`` — Sub-GHz link configuration passed to ``sid_init()``.
  Platform initialization updates its maximum transmit power for the selected region.
* ``iso_country`` — Optional ISO 3166-1 alpha-2 country code, or ``NULL`` if not applicable.

See :ref:`sidewalk_configuration` for more information.

.. _migration_200_libraries:

Prebuilt library layout
=======================

The prebuilt Sidewalk libraries are now located in the :file:`lib/release` and :file:`lib/debug` directories.
The release variant is the default.
If your build system references library paths directly, update those paths or use the ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` and ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig options instead.

Utility headers such as :file:`sid_utils.h` are now located in the :file:`subsys/sal/common/sid_utility_ifc` directory.
The add-on build adds the include path automatically.
Update your project only if it references the old directory directly.

.. _migration_200_api:

Sidewalk API changes
====================

Update application code for the following API changes.
For the complete list, including platform abstraction layer changes, see the `Amazon Sidewalk MCU SDK Migration Guide`_.

* Location Library — Not backward compatible:

  * Renamed ``SID_LOCATION_METHOD_BLE_GATEWAY`` to ``SID_LOCATION_METHOD_GATEWAY``.
  * Replaced the ``buffer`` and ``size`` fields of ``sid_location_run_config`` with ``payload_type`` and ``payload``.
  * Replaced the ``payload``, ``size``, and ``raw_gnss_data`` fields of ``sid_location_result`` with ``result_type`` and ``parsed``.
    Decode results with ``sid_location_parse_wifi()`` and ``sid_location_parse_gnss()``.
  * Removed ``sid_location_update_almanac()``.
  * Changed ``SID_LOCATION_EFFORT_INVALID`` and ``SID_LOCATION_EFFORT_DEFAULT`` when new location levels were added.
    Do not store or compare these values as integers.

  For an example, see the location shell in the :ref:`variant_sidewalk_dut` variant (:file:`samples/sid_end_device/src/cli/location_shell_events.c`).

* Changed ``sid_msg_desc_tx_attributes.resolve_location`` to a bit mask.
  Replace ``true`` with ``SID_TX_LOCATION_LINK_RESOLVE``.

* Added ``gsk_config`` to ``struct sid_config`` for Gateway Session Key (GSK) settings.
  GSK is enabled by default in the v2.0 libraries.
  Use a designated initializer for ``struct sid_config`` and set ``gsk_config`` to ``NULL`` to accept library defaults, as in the :ref:`sidewalk_end_device` sample.

* Added trailing fields to ``struct sid_sub_ghz_links_config`` and ``struct sid_device_profile_unicast_params``.
  Replace positional initializers with designated initializers.

* Changed ``sid_dlrv_config.nwsync_time`` to a 16-bit value.
  Review code that stores or compares it as 8 bits.

* Increased ``SID_BULK_DATA_TRANSFER_FILE_DESCRIPTOR_MAX_SIZE`` from 128 to 200 bytes.
  Review buffers sized with this constant.

* Reserved manufacturing store value identifiers 4001 through 4008 for the Sidewalk SDK.
  Move application data stored under these identifiers elsewhere.

.. _migration_200_kconfig:

Kconfig changes
===============

Renamed the ``CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC`` Kconfig option to ``CONFIG_SIDEWALK_MFG_ALLOW_WRITE``.
The option is still enabled automatically when ``CONFIG_SIDEWALK_ON_DEV_CERT`` is set.
Replace ``CONFIG_SIDEWALK_MFG_STORAGE_DIAGNOSTIC`` with ``CONFIG_SIDEWALK_MFG_ALLOW_WRITE`` in application code.

Troubleshooting
***************

When encountering issues, check the following:

* For linker or boot issues, perform a pristine build:

  .. code-block:: console

     west build -p -b <board_target> <your_application>

* For other issues, ensure all dependencies are up to date:

  .. code-block:: console

     west update

.. note::

   For additional support, refer to the `known issues page on the main branch`_ or consult the Amazon Sidewalk documentation.

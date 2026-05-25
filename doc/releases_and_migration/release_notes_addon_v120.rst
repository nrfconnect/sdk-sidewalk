.. _sidewalk_release_notes_addon_v120:

Release notes for Amazon Sidewalk Add-On v1.2.0
###############################################

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

This release introduces support for nRF54LM20 and nRF54LV10 for Sidewalk (BLE only).

* Added:

  * Support for nrf54lv10 in Sidewalk BLE-only samples.
  * Support for nrf54lm20 in Sidewalk BLE-only samples.
  * BLE adapter callbacks to allow changing BLE parameters at runtime via Sidewalk options.

* Updated:

  * NCS updated from v3.0.0 to v3.3.0.
  * Flash layout: moved from ``pm_static*.yml`` into DTS overlays (except for nrf53, nrf54l10, and non-secure boards).
  * Common BLE device naming and configuration cleanup.

* Fixed:

  * Flash layout on nrf54l10 (manufacturing hex location changes).
  * Crash when running app event engine with WiFi location scan path.
  * Sample: radio initialization failure handling; now bails out on failure instead of continuing.

Backward Compatibility
**********************

.. note::
   **Partition Manager deprecation**
   Nordic is transitioning flash partitioning to Zephyr's default devicetree-based partitioning (DTS), and new Nordic designs are recommended to use DTS instead of Partition Manager.
   For custom board designs, it is strongly recommended to update and validate the Sidewalk partition layout in board DTS overlays during migration from ``pm_static*.yml``.

This release preserves backward compatibility for the application-facing Sidewalk API.

Compatibility exceptions and integration updates:

* nrf54l10 flash layout changed (manufacturing hex location update).
* BLE PAL connection symbols were renamed in ``subsys/sal/sid_pal/include/sid_ble_connection.h``. Applications or integrations that use these PAL symbols directly must update to the new names.

.. note::
   While this release mostly maintains API compatibility, it is recommended to test your applications and board configurations thoroughly after upgrading.

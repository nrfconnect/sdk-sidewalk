.. _sidewalk_release_notes_addon_v110:

Release notes for Amazon Sidewalk Add-On v1.1.0
###############################################

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

This release introduces the latest Amazon Sidewalk SDK v1.19 libraries.



* Added:

  * Latest integration of Amazon Sidewalk v1.19 libraries.
  * **LR1110 radio support** as transport for LoRa and FSK.
  * **DTS (Device Tree) changes** to accommodate LR1110 radio configuration.
  * **New radio architecture** with Zephyr shields for SubGHz radio configuration.
  * **Migration guide** for new radio architecture and shield system.

* Updated:

  * Amazon Sidewalk MCU SDK from v1.18 to v1.19.
  * **Build system** to use Zephyr shields for SubGHz radio configuration.

* Fixed:

  * KRKNWK-19948: ACK parameters mismatch between send function and on_msg_sent JSON output

* Known Issues:

  * Intermittent downlink message reception issues with BLE and FSK (KRKNWK-20869)

* Known Regressions:

  * Boot time increased due to bootloader configuration change (KRKNWK-20863)

Backward Compatibility
**********************

This release maintains backward compatibility with existing applications built for v1.0.x Add-on versions.
It is recommended to initialize new struct fields to known values for better code safety.

.. note::
   While this release maintains API compatibility, it is recommended to test your applications thoroughly
   with the new v1.19 libraries to ensure optimal performance.

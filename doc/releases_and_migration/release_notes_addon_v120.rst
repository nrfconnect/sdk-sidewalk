.. _sidewalk_release_notes_addon_v120:

Release notes for Amazon Sidewalk Add-On v1.2.0
###############################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

This release introduces support for the nRF54LM20 and nRF54LV10 DKs for Sidewalk (Bluetooth LE only).

* Added:

  * Support for the nRF54LV10A SoC in Bluetooth LE-only samples.
  * Support for the nRF54LM20A and nRF54LM20B SoCs in Bluetooth LE-only samples.
  * Bluetooth LE adapter callbacks to allow changing Bluetooth LE parameters at runtime through Sidewalk options.

* Updated:

  * The nRF Connect SDK from v3.0.0 to v3.3.0.
  * Flash layout, by moving :file:`pm_static*.yml` into DTS overlays (except for the nRF53 Series, and non-secure boards).
  * Cleaned up common Bluetooth LE device naming and configuration.
  * On the nRF52 Series: the `Trusted storage`_ default key generation mechanism to ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID`` instead of ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK``.
    The HUK-based option was not fully functional and provided no meaningful security benefit without the `nRF Secure Immutable Bootloader (NSIB)`_.
    Products already deployed in the field must keep the previous key generation setting so that provisioned Sidewalk keys remain accessible; see the :ref:`migration_120_trusted_storage_nrf52` page.

* Fixed:

  * Flash layout on nRF54L10 (changed location of manufacturing HEX file).
  * Sample - radio initialization failures now cause the sample to exit instead of continuing in an invalid state.

Backward compatibility
**********************

.. note::
   Partition Manager deprecation - Nordic Semiconductor is transitioning flash partitioning to Zephyr's default devicetree-based partitioning (DTS), and new Nordic designs are recommended to use DTS instead of Partition Manager.
   For custom board designs, it is strongly recommended to update and validate the Sidewalk partition layout in board DTS overlays during migration from :file:`pm_static*.yml`.

This release preserves backward compatibility for the application-facing Sidewalk API.
For details, see the :ref:`migration_guide_addon_v120` page.

.. warning::
   If your nRF52 Series product uses ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE`` and has Sidewalk keys stored in the trusted storage, do not adopt the new ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID`` default on firmware updates sent to devices already in the field.
   Changing the AEAD key generation mechanism makes existing stored keys unreadable.

.. note::
   While this release mostly maintains API compatibility, ensure to test your applications and board configurations thoroughly after upgrading.

.. _sidewalk_release_notes_addon_changelog:

Release notes for Amazon Sidewalk Add-On v1.2.99
################################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

* Added:

  * Software-based MCUboot downgrade protection (``CONFIG_MCUBOOT_DOWNGRADE_PREVENTION``), used together with overwrite-only upgrade mode.

* Updated:

  * MCUboot signature type to follow the recommended defaults in the nRF Connect SDK.
    On the nRF54L Series platforms, the bootloader now uses ED25519 instead of RSA.
    This reduces MCUboot boot time compared to RSA-based verification.
  * MCUboot configuration to align with NCS recommendations: picolibc and link-time optimization (LTO).
  * Persistent Sidewalk key storage to use Key Management Unit (KMU) by default.
    Use ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU=n`` to keep the default settings-based PSA trusted storage backend.

* Removed:

  * Support for the nRF52 Series (including the nRF52840 DK).
  * Support for the nRF53 Series (including the nRF5340 DK and Thingy:53).

* Fixed:

  * KRKNWK-20863: Increased boot time due to bootloader configuration.

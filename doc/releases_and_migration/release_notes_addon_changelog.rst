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

  * Software-based downgrade protection in MCUboot (``CONFIG_MCUBOOT_DOWNGRADE_PREVENTION``), used together with overwrite-only upgrade mode.
  * Runtime self-locking of the MCUboot flash area (``CONFIG_FPROTECT``) to make the bootloader immutable.
    An immutable first-stage bootloader is an essential component of the secure boot chain.

    .. note::
       On the nRF54L Series platforms, enabling this feature required disabling write protection for the ``mfg_storage`` partition, due to the limited number of configurable protected regions.
       If you lock the bootloader with an alternative method, such as the ``UICR.BOOTCONF`` register, set the Kconfig option ``CONFIG_FPROTECT`` to ``y`` to restore write protection for the ``mfg_storage`` partition.

* Updated:

  * The nRF Connect SDK from v3.3.0 to v3.4.0.
  * Flash layout for the ``nrf54l15dk/nrf54l15/cpuapp/ns`` board target, by completing the migration from Partition Manager to devicetree overlays.
  * MCUboot signature type to follow the recommended defaults in the nRF Connect SDK.
    On the nRF54L Series platforms, the bootloader now uses ED25519 instead of RSA.
    This reduces MCUboot boot time compared to RSA-based verification.
  * MCUboot configuration to align with NCS recommendations: picolibc and link-time optimization (LTO).
  * Persistent Sidewalk key storage to use Key Management Unit (KMU) by default.
    Set the Kconfig option ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU=n`` to keep the default settings-based PSA trusted storage backend.

* Removed:

  * Support for the nRF52 Series (including the nRF52840 DK).
  * Support for the nRF53 Series (including the nRF5340 DK and Thingy:53).

* Fixed:

  * KRKNWK-20863: Increased boot time due to bootloader configuration.

.. _sidewalk_release_notes_addon_v130:

Release notes for Amazon Sidewalk Add-On v1.3.0
###############################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates in Amazon Sidewalk Add-On v1.3.0 as compared to v1.2.0.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

* Added:

  * Software-based downgrade protection in MCUboot (``CONFIG_MCUBOOT_DOWNGRADE_PREVENTION``), used together with overwrite-only upgrade mode.
  * Runtime self-locking of the MCUboot flash area (``CONFIG_FPROTECT``) to make the bootloader immutable.
    An immutable first-stage bootloader is an essential component of the secure boot chain.

    .. note::
       On nRF54L Series platforms, setting ``CONFIG_FPROTECT=y`` in both the bootloader and the application is mutually exclusive due to the limited number of configurable protected regions available in the hardware.
       For this reason, enabling the self-locking feature in MCUboot required disabling locking the ``mfg_storage`` partition from modifications.
       If you lock the bootloader with an alternative method, such as the ``UICR.BOOTCONF`` register, set the Kconfig option ``CONFIG_FPROTECT=y`` in your application to restore write protection for the ``mfg_storage`` partition.

  * Support for the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>` shield on the nRF54L15 DK and nRF54LM20 DK.
    The shield enables the LoRa and FSK link types.
  * :ref:`lr11xx_firmware_update` sample for updating Semtech LR1110 transceiver firmware.
  * :ref:`application_development` documentation section.
  * ``west sid provision`` command for generating the ``mfg_storage`` partition contents and flashing it to a connected board.

* Updated:

  * The nRF Connect SDK from v3.3.0 to v3.4.0.
  * The Amazon Sidewalk libraries to the latest official release v1.19.4.
  * Flash layout for the ``nrf54l15dk/nrf54l15/cpuapp/ns`` board target, by completing the migration from Partition Manager to devicetree overlays.
  * MCUboot signature type to follow the recommended defaults in the nRF Connect SDK.
    On the nRF54L Series platforms, the bootloader now uses ED25519 instead of RSA.
    On the nRF52 Series platforms, the bootloader now uses ECDSA with P-256 curve instead of RSA.
    This reduces MCUboot boot time compared to RSA-based verification.
  * MCUboot configuration to align with NCS recommendations: picolibc and link-time optimization (LTO).
  * Persistent Sidewalk key storage to use Key Management Unit (KMU) by default on nRF54L Series platforms.
    Set the Kconfig option ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU=n`` to keep the default settings-based PSA trusted storage backend.

* Removed:

  * Support for the nRF53 Series (including the nRF5340 DK and Thingy:53).

* Fixed:

  * Long boot time of the device caused by the bootloader configuration. (KRKNWK-20863)
  * Semtech interrupt line remaining in the high state during a Wi-Fi location scan. (KRKNWK-21160)
  * Bluetooth LE advertising failing to start after a Sidewalk deinitialization and reinitialization cycle. (KRKNWK-22208)
  * Failure to initialize location when LoRa or FSK transport is initialized in Amazon Sidewalk. (KRKNWK-20851)
  * Endless loop in GNSS scan. (KRKNWK-20856)
  * Crash in ``sid_location_run`` function due to an invalid context in the Wi-Fi callback. (KRKNWK-21159)

Backward compatibility
**********************

This release preserves backward compatibility for the application-facing Sidewalk API.
For migration details, see the :ref:`migration_guide_addon_v130` page.

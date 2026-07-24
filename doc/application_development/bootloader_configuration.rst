.. _bootloader_configuration:

Bootloader configuration
########################

.. contents::
   :local:
   :depth: 2

This page describes recommended bootloader configuration for Sidewalk products based on nRF Connect SDK.
If you are migrating existing products, refer to the :ref:`release_notes_migration_guides` and :ref:`release_notes` for compatibility information.

Secure bootloader chain
************************

A secure boot chain ensures that only authenticated firmware runs on the device.
Implement one of the following layouts, as described in the `Secure bootloader chain`_ documentation:

* MCUboot only (immutable) - Use non-upgradable MCUboot as the immutable bootloader.
  Enable it with the sysbuild Kconfig option ``SB_CONFIG_BOOTLOADER_MCUBOOT=y``.
* `nRF Secure Immutable Bootloader (NSIB)`_ followed by upgradable MCUboot - This layout allows both application updates and MCUboot updates while keeping an immutable root of trust.
  Enable it with the sysbuild Kconfig options ``SB_CONFIG_SECURE_BOOT_APPCORE=y`` and ``SB_CONFIG_BOOTLOADER_MCUBOOT=y``.

In both layouts, ensure the immutable bootloader is locked from modification by applying one of the following methods:

* Enable the ``CONFIG_FPROTECT`` Kconfig option in the bootloader configuration to self-lock the bootloader area at runtime.
* Use the ``UICR.BOOTCONF`` register to permanently lock the bootloader area on the supported nRF54L Series platforms.

Application image signing
*************************

MCUboot verifies the application image signature before boot.
Configure the signing algorithm and the storage location for verification public keys as described in the following sections.
These settings apply to firmware delivered through any update mechanism, including Sidewalk Device Firmware Update (DFU) (see :ref:`sidewalk_configuration`).

Configuring the signature algorithm
===================================

MCUboot supports multiple signature algorithms for application image verification.
Prefer the Ed25519 signature type for new Sidewalk products.
Enable it with the sysbuild Kconfig option ``SB_CONFIG_BOOT_SIGNATURE_TYPE_ED25519=y``.

Storing public keys in the Key Management Unit
==============================================

Prefer storing verification public keys in the Key Management Unit (KMU) rather than embedding them in the bootloader image.
This approach supports the key revocation policy and keeps the active verification key out of the bootloader binary.

If you also store Sidewalk keys in the KMU, see :ref:`secure_storage` and ensure bootloader and application key slots do not overlap.

Enable the following sysbuild Kconfig options:

* ``SB_CONFIG_MCUBOOT_SIGNATURE_USING_KMU=y``
* ``SB_CONFIG_MCUBOOT_GENERATE_DEFAULT_KEY_FILE=y`` - Generates the default KMU provisioning file from the configured signing key during the build process.

Optionally, configure the following MCUboot image Kconfig options:

* ``CONFIG_BOOT_SIGNATURE_KMU_SLOTS=<count>`` - Configures the number of verification key generations.
* ``CONFIG_BOOT_KEYS_REVOCATION=y`` - Revokes older key generations after a valid update signed with a newer key.

Downgrade protection
********************

Enable downgrade protection so an attacker cannot install older firmware that reintroduces fixed vulnerabilities.
Use one of the following approaches:

* Software-based MCUboot downgrade protection.
  Enable overwrite-only mode (``SB_CONFIG_MCUBOOT_MODE_OVERWRITE_ONLY=y``) and downgrade protection (``CONFIG_MCUBOOT_DOWNGRADE_PREVENTION=y``).
  Set the application image version with ``CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION`` as part of the release process.
* Hardware-based downgrade protection using a monotonic counter.
  Enable downgrade protection with the sysbuild Kconfig option ``SB_CONFIG_MCUBOOT_HARDWARE_DOWNGRADE_PREVENTION=y``.
  Configure the number of available hardware counter slots with ``SB_CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION_COUNTER_SLOTS``, and the current counter value with ``SB_CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION_COUNTER_VALUE``.

.. note::

  The hardware-based approach provides stronger protection against downgrade attacks, but it limits the number of firmware updates to the number of available hardware counter slots.
  Enable it only if the expected number of firmware updates fits within the available counter slots.

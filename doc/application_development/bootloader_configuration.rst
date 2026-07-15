.. _bootloader_configuration:

Bootloader configuration
########################

This page covers the current bootloader configuration recommendations for Sidewalk products based on nRF Connect SDK.
When migrating existing products, check the :ref:`release_notes_migration_guides` and :ref:`release_notes` for compatibility information.

Secure bootloader chain
************************

Implement a secure boot chain with one of the following layouts, as described in the `Secure bootloader chain`_ documentation:

* Non-upgradable MCUboot used as the immutable bootloader.
  Enable MCUboot with the sysbuild Kconfig option ``SB_CONFIG_BOOTLOADER_MCUBOOT=y``.
* An immutable first-stage bootloader, `nRF Secure Immutable Bootloader (NSIB)`_, followed by upgradable MCUboot as the second-stage bootloader.
  This layout allows both application updates and MCUboot updates while keeping an immutable root of trust.
  Enable it with the sysbuild Kconfig options ``SB_CONFIG_SECURE_BOOT_APPCORE=y`` and ``SB_CONFIG_BOOTLOADER_MCUBOOT=y``.

In either case, make sure the first-stage bootloader is locked from modification using one of the following methods:

* Enable the Kconfig option ``CONFIG_FPROTECT`` in the bootloader configuration to self-lock the bootloader area at runtime.
* Use the ``UICR.BOOTCONF`` register to permanently lock the bootloader area on the supported nRF54L Series platforms.

Image signature verification
****************************

Prefer Ed25519 signature type for application image verification in MCUboot.

Enable Ed25519 for MCUboot with the sysbuild Kconfig option ``SB_CONFIG_BOOT_SIGNATURE_TYPE_ED25519=y``.

MCUboot public key storage
**************************

Store public keys used for application image verification in the Key Management Unit (KMU).
This avoids compiling the active verification key directly into the bootloader image and supports the key revocation policy.

Enable the following sysbuild Kconfig options:

* ``SB_CONFIG_MCUBOOT_SIGNATURE_USING_KMU=y``
* ``SB_CONFIG_MCUBOOT_GENERATE_DEFAULT_KEY_FILE=y`` to generate the default KMU provisioning file from the configured signing key at build time.

Optionally, configure the following MCUboot image Kconfig options:

* ``CONFIG_BOOT_SIGNATURE_KMU_SLOTS=<count>`` to support multiple verification key generations.
* ``CONFIG_BOOT_KEYS_REVOCATION=y`` to revoke older key generations after a valid update signed with a newer key.

Downgrade protection
********************

Enable downgrade protection so an attacker cannot install older firmware that reintroduces fixed vulnerabilities.
Use one of the following approaches:

* Software-based MCUboot downgrade protection.
  Enable overwrite-only mode with the sysbuild Kconfig option ``SB_CONFIG_MCUBOOT_MODE_OVERWRITE_ONLY=y`` and enable downgrade checks with the MCUboot Kconfig option ``CONFIG_MCUBOOT_DOWNGRADE_PREVENTION=y``.
  Set the application image version with ``CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION`` as part of the release process.
* Hardware-based downgrade protection using a monotonic counter.
  Enable the downgrade protection with the sysbuild Kconfig option ``SB_CONFIG_MCUBOOT_HARDWARE_DOWNGRADE_PREVENTION=y``.
  Configure the number of available hardware counter slots with the sysbuild Kconfig option ``SB_CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION_COUNTER_SLOTS``, and the current counter value with the sysbuild Kconfig option ``SB_CONFIG_MCUBOOT_HW_DOWNGRADE_PREVENTION_COUNTER_VALUE``.

The hardware-based approach provides stronger protection against downgrade attacks, but it limits the number of firmware updates to the number of available hardware counter slots.
For that reason, it should be enabled with caution, depending on the expected firmware update frequency.

.. _migration_guide_addon_changelog:

Migration guide for Amazon Sidewalk Add-On v1.2.99
##################################################

.. contents::
   :local:
   :depth: 2

.. _migration_changelog_mcuboot_rsa:

Keeping RSA MCUboot signatures
******************************

Earlier add-on releases forced the RSA signature type (``SB_CONFIG_BOOT_SIGNATURE_TYPE_RSA``) for MCUboot across all platforms.
This release removes that override and uses the nRF Connect SDK default (ED25519 on the nRF54L Series platforms).

If your product was based on a previous add-on version, you must keep using RSA for signing application images to stay compatible with MCUboot on already-deployed devices.
To force RSA signature type in your application, add the following to your application's :file:`Kconfig.sysbuild` file:

.. code-block:: kconfig

   choice BOOT_SIGNATURE_TYPE
           default BOOT_SIGNATURE_TYPE_RSA
   endchoice

Perform a pristine build of the application after adding this override.

.. note::
   A device running MCUboot configured for RSA will not boot ED25519-signed images until MCUboot is updated and re-flashed with a matching configuration.

.. note::
   Use ED25519 for new designs on nRF54L Series platforms because of its shorter verification time and smaller key size.

Persistent Sidewalk keys in KMU
*******************************

Persistent Sidewalk cryptographic keys are now stored in the Key Management Unit (KMU) by default on supported nRF54L Series platforms.
Use the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU`` Kconfig option to control this feature.

If you update devices that already have keys stored by an earlier firmware version, set ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU=n`` to keep the keys accessible.
Switching a provisioned product from the PSA trusted storage to KMU changes both the storage location and the PSA key IDs, so Sidewalk cannot find the existing keys.

For new products, use the KMU default.
Make sure that your application does not use the same KMU slots for other keys.
Sidewalk starts at the slot defined by the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU_SLOT_START`` Kconfig option and reserves seven consecutive KMU slots:

* Two slots for the manufacturing ED25519 private key.
* Two slots for the manufacturing secp256r1 private key.
* One slot each for the WAN master, app key, and device-to-device AES keys.

Removed platform support
************************

The upcoming release no longer supports the following development kits:

* nRF52840 DK (``nrf52840dk/nrf52840``)
* nRF5340 DK (``nrf5340dk/nrf5340/cpuapp``)
* Thingy:53 (``thingy53/nrf5340/cpuapp``)

If your product uses one of these platforms, continue using the last add-on release that supports it.

Troubleshooting
***************

#. If you encounter unexpected linker errors or boot issues, perform a pristine build:

   .. code-block:: console

      west build -p -b <board_target> <your_application>

#. Ensure all dependencies are up to date:

   .. code-block:: console

      west update

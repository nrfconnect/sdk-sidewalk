.. _migration_guide_addon_v130:

Migration guide for Amazon Sidewalk Add-On v1.3.0
#################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.2.0 to v1.3.0.

Overview
********

Amazon Sidewalk Add-On v1.3.0 upgrades the nRF Connect SDK to v3.4.0, aligns MCUboot configuration with NCS security recommendations, and moves persistent Sidewalk key storage to Key Management Unit (KMU) on nRF54L Series platforms.

Refer to the :ref:`release_notes_addon_v130` for more details.

Migration steps
***************

#. Navigate to the Amazon Sidewalk Add-on repository (the :file:`sidewalk` directory in your west workspace).

#. Update the Add-on repository to the release tag ``v1.3.0-add-on`` from the `sdk-sidewalk`_ GitHub repository.

   The commands below assume a Git remote named ``ncs`` that points to `sdk-sidewalk`_.
   If your remote uses a different name, substitute it for ``ncs``.

   .. code-block:: console

      git fetch ncs
      git checkout v1.3.0-add-on

#. From your west workspace root (the parent directory of :file:`sidewalk`), update the nRF Connect SDK and other west manifest projects:

   .. code-block:: console

      west update

#. Perform a clean build:

   .. code-block:: console

      west build -p -b <board_target> <sidewalk_application>

.. _migration_130_mcuboot_rsa:

Keeping RSA MCUboot signatures
******************************

Earlier add-on releases forced the RSA signature type (``SB_CONFIG_BOOT_SIGNATURE_TYPE_RSA``) for MCUboot across all platforms.
This release removes that override and uses the nRF Connect SDK defaults (ED25519 on nRF54L Series platforms, ECDSA with P-256 curve on nRF52 Series platforms).

If your product was based on a previous add-on version, you must keep using RSA for signing application images to stay compatible with MCUboot on already-deployed devices.
To force RSA signature type in your application, add the following to your application's :file:`Kconfig.sysbuild` file:

.. code-block:: kconfig

   choice BOOT_SIGNATURE_TYPE
           default BOOT_SIGNATURE_TYPE_RSA
   endchoice

Perform a pristine build of the application after adding this override.

.. note::
   A device running MCUboot configured for RSA will not boot ED25519-signed images until MCUboot is updated and re-flashed with a matching configuration.

.. _migration_130_kmu:

Persistent Sidewalk keys in KMU
*******************************

Persistent Sidewalk cryptographic keys are now stored in the Key Management Unit (KMU) by default on supported nRF54L Series platforms.
Use the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU`` Kconfig option to control this feature.
This option is not available on nRF52 Series platforms, which continue to use the default settings-based PSA trusted storage backend.

If you update devices that already have keys stored by an earlier firmware version, set ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU=n`` to keep the keys accessible.
Switching a provisioned product from the PSA trusted storage to KMU changes both the storage location and the PSA key IDs, so Sidewalk cannot find the existing keys.

For new products, use the KMU default.
Make sure that your application does not use the same KMU slots for other keys.
Sidewalk starts at the slot defined by the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE_KMU_SLOT_START`` Kconfig option and reserves seven consecutive KMU slots:

* Two slots for the manufacturing ED25519 private key.
* Two slots for the manufacturing secp256r1 private key.
* One slot each for the WAN master, app key, and device-to-device AES keys.

.. _migration_130_removed_platforms:

Removed platform support
************************

Amazon Sidewalk Add-On v1.3.0 no longer supports the following development kits:

* nRF5340 DK (``nrf5340dk/nrf5340/cpuapp``)
* Thingy:53 (``thingy53/nrf5340/cpuapp``)

If your product uses one of these platforms, continue using the Sidewalk Add-On version earlier than v1.3.0.

Troubleshooting
***************

When encountering issues, check the following:

* For linker error or boot issues, perform a pristine build:

  .. code-block:: console

     west build -p -b <board_target> <your_application>

* For other issues, ensure all dependencies are up to date:

  .. code-block:: console

     west update

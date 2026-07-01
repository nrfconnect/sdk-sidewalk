.. _migration_guide_addon_changelog:

Migration guide for Amazon Sidewalk Add-On (TBD)
################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.2.0 to (TBD).

Overview
********

TODO: Add overview of changes.

Migration steps
***************

TODO: Add migration summary.

.. _migration_changelog_mcuboot_rsa:

Keeping RSA MCUboot signatures
******************************

Previous add-on releases forced **RSA** signature type for MCUboot on all platforms (``SB_CONFIG_BOOT_SIGNATURE_TYPE_RSA``).
This release removes that override and uses the NCS default (**ED25519** on nRF54L Series platforms).

If your product was developed against a previous add-on version and you must keep using **RSA** for application images signing - for example, to stay compatible with MCUboot on already-deployed devices - add the following to your application's :file:`Kconfig.sysbuild` file:

.. code-block:: kconfig

   choice BOOT_SIGNATURE_TYPE
           default BOOT_SIGNATURE_TYPE_RSA
   endchoice

Perform a pristine build of the application after adding this override.

.. warning::
   A device running MCUboot configured for RSA will not boot ED25519-signed images until MCUboot is updated and re-flashed with a matching configuration.

.. note::
   ED25519 is the recommended option for new designs on nRF54L Series platforms due to its shorter verification time and smaller key size.

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

.. _migration_guide_addon_v120:

Migration guide for Amazon Sidewalk Add-On v1.2.0
##################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.1.0 to v1.2.0, which includes the upgrade to NCS v3.3.0 and the transition from Partition Manager to devicetree-based flash partitioning.

Overview
********

Amazon Sidewalk Add-On v1.2.0 introduces support for three new hardware platforms (nRF54LV10A,  nRF54LM20A and nRF54LM20B) and aligns the flash partition layout with the NCS v3.3.0 transition away from Partition Manager.

Key updates in this release include:

* Including new board support for the nRF54LV10A, nRF54LM20A and nRF45LM20B SoCs in Bluetooth LE-only samples.
* Updating the nRF Connect SDK from v3.0.0 to v3.3.0.
* Migration of flash layout from :file:`pm_static*.yml` files to devicetree overlays for all supported boards except nrf5340 and non-secure variants (see :ref:`migration_120_partition_manager`).
* On the nRF52 Series, changed the default AEAD key of `Trusted storage`_ (see :ref:`migration_120_trusted_storage_nrf52`).
* On the nRF54L10 SoC, corrected application-core RRAM size and relocated the manufacturing storage partition (see :ref:`migration_120_mfg_storage_nrf54l10`).

Migration steps
***************

#. Navigate to the Amazon Sidewalk Add-on repository (the :file:`sidewalk` directory in your west workspace).

#. Update the Add-on repository to the release tag ``v1.2.0-add-on`` from the `sdk-sidewalk`_ GitHub repository.

   The commands below assume a Git remote named ``ncs`` that points to `sdk-sidewalk`_.
   If your remote uses a different name, substitute it for ``ncs``.

   .. code-block:: console

      git fetch ncs
      git checkout v1.2.0-add-on

#. From your west workspace root (the parent directory of :file:`sidewalk`), update the nRF Connect SDK and other west manifest projects:

   .. code-block:: console

      west update

#. Perform a clean build:

   .. code-block:: console

      west build -p -b <board_target> <sidewalk_application>

.. _migration_120_partition_manager:

Partition Manager deprecation
*****************************

Partition Manager has been deprecated starting from nRF Connect SDK v3.3.0.
If your application uses Partition Manager configuration files (for example, :file:`pm_static*.yml` files) to define partition layout, you should migrate to devicetree-based flash partitioning.

Amazon Sidewalk Add-on v1.2.0 moves flash layout from :file:`pm_static*.yml` files to devicetree overlays for supported boards.
Custom board designs should update and validate the Sidewalk partition layout in board DTS overlays during migration.

If you maintain a custom board with a :file:`pm_static*.yml` file, see the `Migrating partition configuration from Partition Manager to devicetree (DTS)`_ page in the NCS documentation.

To confirm that migration succeeded, run a pristine build on your board:

   .. code-block:: console

      west build -p -b <your_board> <sidewalk_application>

.. _migration_120_trusted_storage_nrf52:

Trusted storage on the nRF52 Series
***********************************

With NCS v3.3.0, Amazon Sidewalk Add-on v1.2.0 changes the default `Trusted storage`_ AEAD key backend on the nRF52 Series from the ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK`` to ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID`` Kconfig option.
This applies when Sidewalk PSA key storage is enabled (``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE``).

Choose the backend based on whether devices already store Sidewalk keys in trusted storage:

* Field firmware updates - Keep the ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK`` Kconfig option enabled so that existing trusted storage entries remain readable.
  The HUK partition contents do not matter for this backend unless you use the `nRF Secure Immutable Bootloader (NSIB)`_.

* New products and factory-fresh devices - You can either keep the new default (``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID``) or explicitly select the ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK`` Kconfig option.
  The HUK-derived option can offer stronger key binding, but requires NSIB to load the HUK partition into the CryptoCell and adds a bootloader stage.

For background on Sidewalk key storage, see the Secure Key Storage section in :ref:`sidewalk_end_device` and the `Trusted storage`_ documentation in the nRF Connect SDK.

.. warning::
   After Sidewalk keys are written to trusted storage, changing the ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_*`` Kconfig option or other trusted-storage crypto settings makes existing entries unreadable and can brick in-field devices.

.. _migration_120_mfg_storage_nrf54l10:

Manufacturing storage on the nRF54L10 device
********************************************

Amazon Sidewalk Add-on v1.2.0 corrects the application-core RRAM map on the nRF54L10 SoC and moves Sidewalk manufacturing data out of Partition Manager into devicetree.

In Add-on v1.1.0, Partition Manager placed the ``mfg_storage`` region at ``0xFF000`` (4 KB), based on a 1024 KB RRAM assumption.
The SoC exposes 1012 KB of application-core RRAM, so that address was past the end of physical memory.
The in-tree DTS overlay now defines ``mfg_storage`` at ``0xFC000`` (4 KB), at the end of the 1012 KB RRAM region.
The PAL reads this partition through ``FIXED_PARTITION_OFFSET(mfg_storage)`` when Partition Manager is disabled.

Complete the following steps:

#. Update your board devicetree overlay to match the Add-on layout: ``cpuapp_rram`` size 1012 KB and an ``mfg_storage`` partition at ``0xFC000`` (4 KB, label ``mfg_storage``).
   See :file:`samples/sid_end_device/boards/nrf54l15dk_nrf54l10_cpuapp.overlay` for the reference partition map.

#. Regenerate and flash the manufacturing image using ``--addr 0xFC000`` (not ``0xFF000``) in :file:`provision.py``.
   Full command examples are in the :ref:`setting_up_sidewalk_prototype` page (nRF54L10 tab).

#. Perform a pristine application build and flash the updated firmware.

#. If devices were previously provisioned with a HEX file built for ``0xFF000``, reprovision with the new HEX and factory-reset the device so Sidewalk registration uses the data at the new address.

Troubleshooting
***************

#. If you encounter unexpected partition or linker errors, perform a pristine build:

   .. code-block:: console

      west build -p -b <board_target> <your_application>

#. Ensure all dependencies are up to date:

   .. code-block:: console

      west update

#. Verify that ``SB_CONFIG_PARTITION_MANAGER`` is set correctly for your build configuration.

.. note::

   For additional support, refer to the :ref:`known_issues` page or consult the Amazon Sidewalk documentation.

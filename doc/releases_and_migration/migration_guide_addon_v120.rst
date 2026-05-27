.. _migration_guide_addon_v120:

Migration guide for Amazon Sidewalk Add-On v1.2.0
##################################################

.. contents::
   :local:
   :depth: 2

This guide assists you in migrating from Amazon Sidewalk Add-On v1.1.0 to v1.2.0, which includes the upgrade to NCS v3.3.0 and the transition from Partition Manager to devicetree-based flash partitioning.

Overview
********

Amazon Sidewalk Add-On v1.2.0 introduces support for two new hardware platforms (nRF54LV10 and nRF54LM20) and aligns the flash partition layout with the NCS v3.3.0 transition away from Partition Manager.

Key updates in this release include:

* Including new board support for the nRF54LV10 and nRF54LM20 SoCs in Bluetooth LE-only samples.
* Migration of flash layout from :file:`pm_static*.yml` files to devicetree overlays for supported boards.
* Updating the nRF Connect SDK from v3.0.0 to v3.3.0.
* On the nRF52 Series, a new default for `Trusted storage`_ key provider.

Migration steps
***************

#. Update your west configuration to use the latest Add-on version:

   .. code-block:: console

      west update

#. Perform a clean build:

   .. code-block:: console

      west build -p -b <board_target> <your_application>

#. If you maintain a custom board with a :file:`pm_static*.yml` file, see the `Migrating partition configuration from Partition Manager to devicetree (DTS)`_ page in the NCS documentation.

Backward compatibility
**********************

This release preserves backward compatibility for the application-facing Sidewalk API, with the following exceptions:

* nRF54L10 flash layout — Updated the manufacturing HEX file location.
  Custom board configurations must update their DTS overlays accordingly.
* Bluetooth LE PAL connection symbols has been renamed in :file:`subsys/sal/sid_pal/include/sid_ble_connection.h``.
  Applications that use these PAL symbols directly must update to the new names.
* nRF52 trusted storage default key provider changed to ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID``.

  * Firmware for already deployed devices must retain ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK`` in order to keep existing keys stored in the trusted storage accessible.
    The contents of the HUK partition for this setting is irrelevant as long as the nRF Secure Immutable Bootloader (NSIB) is not used.
  * New products and factory-fresh devices may use the NCS default ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_HASH_UID``, or use ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_DERIVE_FROM_HUK``.
    In the latter case, the nRF Secure Immutable Bootloader (NSIB) must be used in order to load the HUK partition contents into the CryptoCell peripheral.
    This option provides a better security posture than the default option but requires another bootloader stage.

.. warning::
   Once keys are stored in the trusted storage, changing ``CONFIG_TRUSTED_STORAGE_BACKEND_AEAD_KEY_*`` (or other trusted-storage crypto settings) makes existing entries unreadable.
   See the Secure Key Storage section in :ref:`sidewalk_end_device`.

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

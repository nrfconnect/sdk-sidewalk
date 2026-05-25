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

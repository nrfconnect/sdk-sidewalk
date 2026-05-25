.. _migration_guide_addon_v120:

Migration guide for Amazon Sidewalk Add-On v1.2.0
##################################################

This guide assists you in migrating from Amazon Sidewalk Add-On v1.1.0 to v1.2.0, which includes the upgrade to NCS v3.3.0 and the transition from Partition Manager to devicetree-based flash partitioning.

Overview
********

Amazon Sidewalk Add-On v1.2.0 introduces support for two new hardware platforms (nRF54LV10 and nRF54LM20) and aligns the flash partition layout with the NCS v3.3.0 transition away from Partition Manager.

Key updates in this release include:

* New board support for nRF54LV10 and nRF54LM20 in Sidewalk BLE-only samples.
* Flash layout migrated from ``pm_static*.yml`` files to devicetree overlays for supported boards.
* NCS updated from v3.0.0 to v3.3.0.

Migration Steps
***************

#. Update your west configuration to use the latest Add-on version:

   .. code-block:: console

      west update

#. Perform a clean build:

   .. code-block:: console

      west build -p -b <board_target> <your_application>

#. If you maintain a custom board with a ``pm_static*.yml`` file, follow the `Migrating partition configuration from Partition Manager to devicetree (DTS) <https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/releases_and_maturity/migration/migration_partitions.html>`_ guide in the NCS documentation.

Backward Compatibility
**********************

This release preserves backward compatibility for the application-facing Sidewalk API, with the following exceptions:

* **nRF54L10 flash layout changed** — manufacturing hex location updated.
  Custom board configurations must update their DTS overlays accordingly.
* **BLE PAL connection symbols renamed** in ``subsys/sal/sid_pal/include/sid_ble_connection.h``.
  Applications that use these PAL symbols directly must update to the new names.

Troubleshooting
***************

#. Perform a pristine build if you encounter unexpected partition or linker errors:

   .. code-block:: console

      west build -p -b <board_target> <your_application>

#. Ensure all dependencies are up to date:

   .. code-block:: console

      west update

#. Verify that ``SB_CONFIG_PARTITION_MANAGER`` is set correctly for your build configuration.

.. note::

   For additional support, refer to the :ref:`known_issues` page or consult the Amazon Sidewalk documentation.

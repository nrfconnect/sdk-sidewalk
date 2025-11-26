.. _migration_guide_addon_v110:

Migration guide for Amazon Sidewalk Add-On v1.1.0
##################################################

This guide assists you in migrating from Amazon Sidewalk Add-On v1.0.x to v1.1.0, which includes the upgrade to Amazon Sidewalk SDK v1.19.

Overview
********

Amazon Sidewalk Add-On v1.1.0 introduces support for the latest Amazon Sidewalk SDK v1.19 and brings several important changes to radio configuration and hardware support.

Key updates in this release include:

* Upgrading to Amazon Sidewalk SDK v1.19, which adds new features and updates to the API.
* Adopting a new radio architecture that uses Zephyr shields for Sub-GHz radio configuration, simplifying hardware integration.
* Enabling hardware abstraction by automatically configuring device tree settings through the Zephyr shield system.
* Experiencing performance regressions in some configurations, including increased boot time due to changes in the bootloader.

Migration Steps
***************

#. Update your west configuration to use the latest Add-on version:

   .. code-block:: console

      west update

#. (Recommended) Update your application code.
   Initialize new fields in your ``sid_config`` structure to NULL for better code safety:

   .. code-block:: c

      struct sid_config config = {
         .link_mask = persistent_link_mask,
         .dev_ch = dev_ch,
         .callbacks = &event_callbacks,
         .link_config = app_get_ble_config(),
         .sub_ghz_link_config = app_get_sub_ghz_config(),
         .log_config = NULL,        // NEW: Must be set to NULL
         .time_sync_config = NULL,  // NEW: Must be set to NULL
      };

#. Update build configuration for Sub-GHz support (if applicable).
   For applications using SubGHz (LoRa and FSK) radios, use the new shield-based build commands:

   .. code-block:: console

      # For SX1262 shield on nRF52840
      west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas

      # For LR1110 shield on nRF52840
      west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

      # For SX1262 shield on nRF54L15
      west build -b nrf54l15dk/nrf54l15/cpuapp samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas

      # For LR1110 shield on nRF54L15
      west build -b nrf54l15dk/nrf54l15/cpuapp samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

#. Test your application with the new v1.19 libraries to ensure optimal performance.

New Radio Architecture
**********************

The v1.1.0 release introduces a new radio architecture based on Zephyr shields that provides better hardware abstraction and automatic device tree configuration for Sub-GHz radios:

* Automatic DTS configuration - Shields automatically configure device tree overlays with correct pin mappings.
* Hardware abstraction - No need to manually specify GPIO pins or SPI configurations.
* Kconfig integration - Shield selection automatically enables the correct radio type in Kconfig.
* Multi-shield support - Use multiple shields (for example, Arduino adapter with radio shield).
* Simplified build process - Single command builds with automatic configuration.

The following shields are supported:

* ``simple_arduino_adapter`` - Arduino header adapter for nRF52840 and nRF54L15 DK
* ``semtech_sx1262mb2cas`` - SX1262 LoRa radio shield
* ``semtech_lr1110mb1xxs`` - LR1110 LoRa radio shield

Build configuration examples
****************************

The following examples demonstrate how to configure and build the sample for different radio and library combinations.

Bluetooth LE-Only Configuration (minimal libraries)
===================================================

For applications that only need Bluetooth LE transport, run the following command:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device -- -DCONFIG_SID_END_DEVICE_DUT=y

This will result in the following configuration:

* Libraries - Minimal Bluetooth LE-only Sidewalk libraries
* Transport - Bluetooth LE only

SX1262 Configuration (Full Libraries with LoRa/FSK)
===================================================

For applications requiring LoRa and FSK transport with SX1262 radio, run the following command:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas

This will result in the following configuration:

* Libraries - Full Sidewalk libraries (Bluetooth LE, LoRa, and FSK)
* Transport - Bluetooth LE, LoRa, FSK
* Radio - SX1262 LoRa transceiver

LR1110 Configuration (Full Libraries)
=====================================

For applications requiring LoRa/FSK transport with LR1110 radio, run the following command:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

This will result in the following configuration:

* Libraries - Full Sidewalk libraries (Bluetooth LE, LoRa, and FSK)
* Transport - Bluetooth LE, LoRa, FSK
* Radio - LR1110 LoRa transceiver

Library Comparison Summary:
===========================

+------------------+------------------+------------------+
| Configuration    | BLE Libraries    | SubGHz Libraries |
+==================+==================+==================+
| Bluetooth LE-Only| ✓                | ✗                |
+------------------+------------------+------------------+
| SX1262           | ✓                | ✓ (LoRa/FSK)     |
+------------------+------------------+------------------+
| LR1110           | ✓                | ✓ (LoRa/FSK)     |
+------------------+------------------+------------------+

API Changes for Developers
**************************

This section details significant API changes between Sidewalk SDK v1.18 and v1.19 that you need to be aware of when integrating the new version.

Enhanced configuration structure
================================

The ``sid_config`` structure has been enhanced with new optional fields:

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``
* The following, new fields:

   * ``const struct sid_diag_log_config *log_config`` - Diagnostic logging configuration
   * ``const struct sid_time_sync_config *time_sync_config`` - Time synchronization configuration

* New supporting structure:

   * ``struct sid_time_sync_config`` - Adaptive time sync intervals configuration

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_time_sync_config.h``

New configuration options
=========================

Two new options have been added to the ``sid_option`` enum:

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``
* Options:

   * ``SID_OPTION_DL_ROUTE_VALIDITY = 19`` - Configure downlink route validity settings
   * ``SID_OPTION_SUB_GHZ_USER_CONTROL = 20`` - Set user control for SubGHz operations

* New supporting structures:

   * ``struct sid_dlrv_config`` - Downlink route validity configuration
   * ``struct sid_link_type_2_gw_discovery_policy_config`` - Gateway discovery policy

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_900_cfg.h``

Enhanced control events
=======================

New control event types have been added:

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``
* New event: ``SID_CONTROL_EVENT_DL_ROUTE_VALIDITY_CONFIG_UPDATE = 3`` (DL route validity configuration changes)

New gateway discovery policies
==============================

Enhanced gateway discovery options for FSK (Link type 2):

* File path: ``sidewalk/subsys/sal/common/sid_ifc/sid_900_cfg.h``

* New policies:

   * ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_FAST_CONNECTION = 1``
   * ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_RELIABLE_CONNECTION = 2``
   * ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_POWER_SAVE = 3``
   * ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_CUSTOM = 4``

Backward compatibility
======================

The following points outline the backward compatibility considerations when migrating from Sidewalk SDK v1.18 to v1.19.

* API compatibility:

   * All existing function signatures remain unchanged.
   * Existing data structures are preserved.
   * All v1.18 code will compile without modification.

* Migration requirements:

   * Code changes required – New fields must be explicitly initialized to avoid crashes.
   * New features are opt-in but require proper initialization to maintain v1.18 behavior.

* Integration notes:

   * Time sync configuration is optional (defaults used if not provided)
   * New options are backward compatible (can be ignored by existing code)

Troubleshooting
***************

This section provides steps to help resolve common issues that may occur during migration to the new SDK version.

1. Perform a clean build of your application:

   .. parsed-literal::
      :class: highlight

      west build -b *board_target* *your_application* --pristine

#. Ensure all dependencies are properly updated:

   .. code-block:: console

      west update

#.  Verify that your application configuration is compatible with the new SDK version.

.. note::

   For additional support, refer to the :ref:`known_issues` page or consult the Amazon Sidewalk documentation.

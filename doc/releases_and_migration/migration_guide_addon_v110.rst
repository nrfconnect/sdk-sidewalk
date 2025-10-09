.. _migration_guide_addon_v110:

Migration guide for Amazon Sidewalk Add-On v1.1.0
##################################################

This guide assists you in migrating from Amazon Sidewalk Add-On v1.0.x to v1.1.0, which includes the upgrade to Amazon Sidewalk SDK v1.19.

Overview
********

Amazon Sidewalk Add-On v1.1.0 introduces the latest Amazon Sidewalk SDK v1.19 libraries with a new radio architecture based on Zephyr shields for SubGHz radio configuration.

What's New in v1.1.0
*********************

* **Amazon Sidewalk SDK v1.19**: Latest SDK version with new features and API updates
* **New Radio Architecture**: Zephyr shields for SubGHz radio configuration
* **Hardware Abstraction**: Automatic DTS configuration with shield system
* **Performance Regressions**: Boot time increased on some configurations

Migration Requirements
**********************

.. note::
   **Recommended:** It is recommended to set all struct fields to known values for better code safety.
   New fields in the ``sid_config`` structure should be initialized to NULL to avoid potential issues.

Migration Steps
***************

**Step 1: Update your Add-on version**

Update your west configuration to use the latest Add-on version:

.. code-block:: console

   west update

**Step 2: Update your application code (RECOMMENDED)**

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

**Step 3: Update build configuration for SubGHz support (if applicable)**

For applications using SubGHz (LoRa/FSK) radios, use the new shield-based build commands:

.. code-block:: console

   # For SX1262 shield on nRF52840
   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas
   
   # For LR1110 shield on nRF52840 
   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs
   
   # For SX1262 shield on nRF54L15
   west build -b nrf54l15dk/nrf54l15/cpuapp samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas
   
   # For LR1110 shield on nRF54L15
   west build -b nrf54l15dk/nrf54l15/cpuapp samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

**Step 4: Test your application**

Thoroughly test your application with the new v1.19 libraries to ensure optimal performance.

New Radio Architecture
***********************

The v1.1.0 release introduces a new radio architecture based on Zephyr shields that provides better hardware abstraction and automatic device tree configuration for SubGHz radios.

**Key Benefits:**

* **Automatic DTS Configuration**: Shields automatically configure device tree overlays with correct pin mappings
* **Hardware Abstraction**: No need to manually specify GPIO pins or SPI configurations  
* **Kconfig Integration**: Shield selection automatically enables the correct radio type in Kconfig
* **Multi-shield Support**: Use multiple shields (e.g., Arduino adapter + radio shield)
* **Simplified Build Process**: Single command builds with automatic configuration


**Supported Shields:**

* ``simple_arduino_adapter`` - Arduino header adapter for nRF52840 and nRF54L15
* ``semtech_sx1262mb2cas`` - SX1262 LoRa radio shield
* ``semtech_lr1110mb1xxs`` - LR1110 LoRa radio shield

Build Configuration Examples
****************************

**BLE-Only Configuration (Minimal Libraries)**
==============================================

For applications that only need BLE transport:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device -- -DCONFIG_SID_END_DEVICE_DUT=y

**Resulting Configuration:**

* **Libraries**: Minimal BLE-only Sidewalk libraries
* **Transport**: BLE only

**SX1262 Configuration (Full Libraries with LoRa/FSK)**
======================================================

For applications requiring LoRa and FSK transport with SX1262 radio:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_sx1262mb2cas

**Resulting Configuration:**

* **Libraries**: Full Sidewalk libraries (BLE + LoRa + FSK)
* **Transport**: BLE, LoRa, FSK
* **Radio**: SX1262 LoRa transceiver

**LR1110 Configuration (Full Libraries)**
=========================================

For applications requiring LoRa/FSK transport with LR1110 radio:

.. code-block:: console

   west build -b nrf52840dk/nrf52840 samples/sid_end_device --shield simple_arduino_adapter --shield semtech_lr1110mb1xxs

**Resulting Configuration:**

* **Libraries**: Full Sidewalk libraries (BLE + LoRa + FSK)
* **Transport**: BLE, LoRa, FSK
* **Radio**: LR1110 LoRa transceiver

**Library Comparison Summary:**
==============================

+------------------+------------------+------------------+
| Configuration    | BLE Libraries    | SubGHz Libraries |
+==================+==================+==================+
| BLE-Only         | ✓                | ✗                |
+------------------+------------------+------------------+
| SX1262           | ✓                | ✓ (LoRa/FSK)     |
+------------------+------------------+------------------+
| LR1110           | ✓                | ✓ (LoRa/FSK)     |
+------------------+------------------+------------------+

API Changes for Developers
**************************

This section details the significant API changes between Sidewalk SDK v1.18 and v1.19 that developers need to be aware of when integrating the new version.

**Enhanced Configuration Structure**
===================================

The ``sid_config`` structure has been enhanced with new optional fields:

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``

**New Fields Added:**

* ``const struct sid_diag_log_config *log_config`` - Diagnostic logging configuration
* ``const struct sid_time_sync_config *time_sync_config`` - Time synchronization configuration

**New Supporting Structure:**

* ``struct sid_time_sync_config`` - Adaptive time sync intervals configuration

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_time_sync_config.h``

**New Configuration Options**
============================

Two new options have been added to the ``sid_option`` enum:

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``

**New Options:**

* ``SID_OPTION_DL_ROUTE_VALIDITY = 19`` - Configure downlink route validity settings
* ``SID_OPTION_SUB_GHZ_USER_CONTROL = 20`` - Set user control for SubGHz operations

**New Supporting Structures:**

* ``struct sid_dlrv_config`` - Downlink route validity configuration
* ``struct sid_link_type_2_gw_discovery_policy_config`` - Gateway discovery policy

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_900_cfg.h``

**Enhanced Control Events**
===========================

New control event types have been added:

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_api.h``

**New Events:**

* ``SID_CONTROL_EVENT_DL_ROUTE_VALIDITY_CONFIG_UPDATE = 3`` - DL route validity configuration changes

**New Gateway Discovery Policies**
=================================

Enhanced gateway discovery options for FSK (Link Type 2):

**File Path:** ``sidewalk/subsys/sal/common/sid_ifc/sid_900_cfg.h``

**New Policies:**

* ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_FAST_CONNECTION = 1``
* ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_RELIABLE_CONNECTION = 2``
* ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_OPTIMIZED_FOR_POWER_SAVE = 3``
* ``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_CUSTOM = 4``

**Backward Compatibility**
=========================

**API Compatibility:**

* All existing function signatures remain unchanged
* Existing data structures are preserved
* All v1.18 code will compile without modification

**Migration Requirements:**

* **Code changes required** - New fields must be explicitly initialized to avoid crashes
* New features are opt-in but require proper initialization to maintain v1.18 behavior

**Integration Notes:**

* Time sync configuration is optional (defaults used if not provided)
* New options are backward compatible (can be ignored by existing code)

Troubleshooting
***************

If you encounter issues during migration:

1. **Clean build**: Perform a clean build of your application:

   .. code-block:: console

      west build -b *board_target* *your_application* --pristine

2. **Verify dependencies**: Ensure all dependencies are properly updated:

   .. code-block:: console

      west update

3. **Check configuration**: Verify that your application configuration is compatible with the new SDK version.

For additional support, refer to the :ref:`known_issues` page or consult the Amazon Sidewalk documentation.

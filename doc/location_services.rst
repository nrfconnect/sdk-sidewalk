.. _location_services:

Location services
##################

.. contents::
   :local:
   :depth: 2

The Amazon Sidewalk Location Library provides an API that automatically selects the most power- and time-efficient location mechanism based on available hardware and network conditions.

Overview
********

The Amazon Sidewalk Location Library is available in the Amazon Sidewalk SDK v1.19 and later, and it enables Sidewalk customers to use a range of cloud-based location solvers.
It also provides better control to configure and override the mechanism used to determine location as required.

The Amazon Sidewalk SDK v2.0 extends the library with the following features:

* FSK gateway location - Resolves the device location from the FSK gateway it is connected to.
* LoRa multilateration (trilateration) - Resolves the device location from a LoRa uplink received by several Sidewalk gateways, without any additional location hardware on the device.
* Caller-defined effort levels - The application can provide its own ordered list of location levels to try.
* Typed location results - The Wi-Fi and GNSS scan results are delivered as typed structures, with helper functions to parse them.

Supported Location Methods
===========================

The location library supports the following location resolution methods:

* Sidewalk Network Location over Bluetooth LE - Uses existing Bluetooth LE connections to determine location through the Sidewalk network.
* Sidewalk Network Location over FSK - Uses the existing FSK connection to determine location from the position of the connected gateway.
* LoRa multilateration - Uses a LoRa uplink received by multiple gateways to compute the device position in the cloud.
* Wi-Fi Scan - Scans for nearby Wi-Fi access points and sends MAC addresses to the cloud for location resolution.
* GNSS Scan - Uses Global Navigation Satellite System to collect satellite vehicle data for coordinate resolution.

The Bluetooth LE, FSK and LoRa multilateration methods do not send any measurement from the device.
The device only signals the Sidewalk network, and the position is resolved in the cloud.
The Wi-Fi and GNSS methods send the scan results over Bluetooth LE or LoRa.

.. note::
   FSK with Wi-Fi and GNSS scanning enabled is not currently supported.

GNSS scanning uses the Semtech LoRa Basics Modem middleware to manage and perform scans on the device, and requires the LR1110 radio.
Wi-Fi scanning can use either the LR1110 with the same middleware, or an nRF70 Series Wi-Fi companion IC.
The two implementations are mutually exclusive, so only one of them can be enabled in a build.
For more information about the nRF70 Series-based Wi-Fi location, see :ref:`location_services_wifi_nrf70`.

.. list-table:: Hardware and Library Requirements for Location Methods
   :header-rows: 1

   * - **Location Method**
     - **Required Hardware**
     - **Supported Sidewalk Library Variants**
   * - BLE Location
     - nRF chip
     - BLE only, or LoRa/FSK
   * - FSK Gateway Location
     - nRF chip and sub-GHz radio (SX1262 or LR1110)
     - LoRa and FSK
   * - LoRa Multilateration
     - nRF chip and sub-GHz radio (SX1262 or LR1110)
     - LoRa and FSK
   * - Wi-Fi Location (LR1110)
     - nRF chip and LR1110 radio
     - LoRa and FSK
   * - Wi-Fi Location (nRF70 Series)
     - nRF chip and nRF70 Series Wi-Fi companion IC
     - BLE only, or LoRa/FSK
   * - GNSS Location
     - nRF chip and LR1110 radio
     - LoRa and FSK

.. note::
   Semtech SX1262 radio does not support Wi-Fi and GNSS scans.
   However, you can still use the Bluetooth LE, FSK gateway, and LoRa multilateration location methods while using the SX1262 for LoRa and FSK communication.
   To add Wi-Fi-based location, use an nRF70 Series Wi-Fi companion IC.

Location Levels
===============

Each location method has its own location level (L1 to L5).
The level number identifies the method, it is not a ranking of power consumption.
The available hardware and links on the device determine which levels can be used.

By default, the library tries the levels in ascending order and moves to the next level when a level is not available, fails, or times out.
Since the Amazon Sidewalk SDK v2.0, you can also provide an ordered list of levels in the ``levels`` and ``num_levels`` fields of the ``sid_location_run_config`` structure.
The library then tries only the listed levels, in the given order.

The library only uses the methods that are enabled in the ``sid_location_type_mask`` field of the ``sid_location_config`` structure (for example, ``SID_LOCATION_METHOD_FSK_GATEWAY`` or ``SID_LOCATION_METHOD_LORA_MLAT``), up to the level set in the ``max_effort`` field.

The following table lists the location levels:

.. list-table:: Location levels
   :header-rows: 1

   * - **Level**
     - **Method**
     - **Required link**
     - **Resolved by**
   * - L1
     - Bluetooth LE gateway location
     - Bluetooth LE
     - Amazon Sidewalk, from the gateway position
   * - L2
     - FSK gateway location
     - FSK
     - Amazon Sidewalk, from the gateway position
   * - L3
     - Wi-Fi scan
     - Bluetooth LE or LoRa
     - AWS IoT Wireless, from an access point database
   * - L4
     - GNSS scan
     - Bluetooth LE or LoRa
     - AWS IoT Wireless, from satellite data
   * - L5
     - LoRa multilateration
     - LoRa
     - Amazon Sidewalk, from the gateways that received the uplink

For details on how each level is resolved, see `Amazon Sidewalk Location Resolution`_.

For more details on integrating device location services with Amazon Sidewalk, refer to the AWS post `Introducing AWS IoT Core Device Location Integration with Amazon Sidewalk`_

This article provides guidance on enabling Device Location for Sidewalk devices, configuring geolocation options in AWS IoT Core, and understanding how location data is transmitted and resolved within the AWS ecosystem.

Level 1: Connected to Sidewalk through BLE
------------------------------------------

At this level, the device uses its existing Sidewalk Bluetooth LE connection to resolve location.

* Power Consumption - No additional power used
* Description - If the device can be located through Bluetooth LE over the Sidewalk network, the device notifies the cloud to resolve the location without extra effort.
  If the device is not connected through Bluetooth LE or a location cannot be resolved, the library moves to the next level.
  The device’s location is based on the Amazon Hub it is connected to.

Level 2: Connected to Sidewalk through FSK
------------------------------------------

At this level, the device uses its existing Sidewalk FSK connection to resolve location.

* Power Consumption - No additional power used
* Description - Works the same way as Level 1, but over the FSK link.
  The device location is the known position of the gateway that received the FSK uplink, so the accuracy is limited to the gateway granularity.
  If the device is not connected through FSK, the library moves to the next level.

Level 3: Send Wi-Fi Scan
------------------------

At this level, the device tries to find its location using nearby Wi-Fi networks.

* Power Consumption - Low
* Description - The device scans for nearby Wi-Fi access points and sends the results to the cloud.
  It must find at least one access point.
  Otherwise, the device will move to the next level.
  If the cloud returns a low confidence score or cannot determine the location, the application should proceed to the next level.

Level 4: Send GNSS Scan
-----------------------

At this level, the device uses satellite data for location.

* Power Consumption - Higher power consumption than Wi-Fi scanning
* Description - The device collects and sends GNSS (satellite) data.
  At least four satellites must be detected for a valid scan.

.. _location_services_lora_mlat:

Level 5: LoRa multilateration
-----------------------------

At this level, the device location is computed from a single LoRa uplink received by several Sidewalk gateways.
This is often called trilateration.

* Power Consumption - No scan is performed, the device sends a single LoRa uplink
* Description - The device sends a LoRa uplink that does not require a network-synchronized connection to one gateway.
  The cloud computes the position from the receptions of this uplink by at least three gateways.
  The accuracy depends on the number and the geometry of the gateways that receive the uplink.
  If fewer than three gateways receive the uplink, the location cannot be resolved.

To use this level, enable the LoRa link (``SID_LINK_TYPE_3``) in the Sidewalk configuration, and add ``SID_LOCATION_METHOD_LORA_MLAT`` to the ``sid_location_type_mask`` field with ``max_effort`` set to ``SID_LOCATION_EFFORT_L5`` or higher.

.. _location_services_wifi_nrf70:

Wi-Fi scanning with the nRF70 Series
************************************

As an alternative to the LR1110 radio, you can use an nRF70 Series Wi-Fi companion IC to perform the Wi-Fi scans from which the location service resolves the device position.
Support is enabled automatically when you build with an nRF7002 EB II shield, which selects the ``CONFIG_SIDEWALK_NRF7X_LOCATION`` Kconfig option.
For the list of supported shield variants, see :ref:`sid_end_device_wifi_location_hw`.

Wi-Fi scanning with the nRF70 Series has the following limitations:

* GNSS is not available, because the nRF70 Series does not include a GNSS receiver.
  Only the Wi-Fi location level can be resolved.
* The nRF70 Series and the LR1110 are mutually exclusive as scanning sources.
  The nRF70 Series is therefore unavailable when the LR1110 is selected as the sub-GHz radio, which includes builds for the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`.

The companion IC runs in the scan-only mode of the nRF70 driver (the ``CONFIG_NRF70_SCAN_ONLY`` Kconfig option, enabled by default for this feature), so it provides Wi-Fi scanning only, without Wi-Fi connectivity.

To select the Wi-Fi scan type, use the ``SIDEWALK_NRF7X_LOCATION_SCAN_TYPE`` Kconfig choice:

* ``CONFIG_SIDEWALK_NRF7X_LOCATION_SCAN_ACTIVE`` -- Default option.
  Discovers access points faster, at the cost of higher power consumption.
* ``CONFIG_SIDEWALK_NRF7X_LOCATION_SCAN_PASSIVE`` -- Discovers access points more slowly, but with lower power consumption.

Testing
*******

The Sidewalk location feature is implemented in the DUT application.
For testing instructions, see :ref:`variant_sidewalk_dut_test_location`.

Sidewalk libraries
==================

Location service is supported in Sidewalk libraries in the following range:

* Sidewalk Sub-GHz library (LoRa and FSK) supports all location methods.
  The FSK gateway location and LoRa multilateration methods work with both the SX1262 and LR1110 radios.
  However to build with radio and pal componets for WiFi and GNSS scanning, ``CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110`` must be enabled.
  This config is enabled automatically when sample is build with ``semtech_lr11xxmb1xxs`` shield.
  Alternatively, the Wi-Fi scanning alone can be provided by an nRF70 Series companion IC, as described in :ref:`location_services_wifi_nrf70`.

* Sidewalk Bluetooth LE only library supports the network location method over Bluetooth LE, and the Wi-Fi location method when an nRF70 Series companion IC is used.

Writing custom application
**************************

For detailed API documentation and usage guide for developers, refer to the `Amazon Sidewalk Location Library Developer Guide`_.

Location resolution with a user message
=======================================

Besides calling ``sid_location_run()``, you can request the location resolution together with a regular uplink message.
To do this, set the ``resolve_location`` field of the message descriptor transmit attributes passed to ``sid_put_msg()``:

.. code-block:: c

   struct sid_msg_desc desc = {
       .type = SID_MSG_TYPE_NOTIFY,
       .link_type = SID_LINK_TYPE_ANY,
       .msg_desc_attr = {
           .tx_attr = {
               .resolve_location = SID_TX_LOCATION_LINK_RESOLVE,
           },
       },
   };

   sid_error_t ret = sid_put_msg(handle, &msg, &desc);

The cloud resolves the device location from the known position of the gateway that receives the message, without a separate location uplink.
Before sending such a message, initialize the Location Library with ``sid_location_init()`` and make sure Sidewalk is ready.
The Location Library reports the delivery status through its ``on_update`` callback, with the ``SID_LOCATION_RESOLVE_MSG_SENT``, ``SID_LOCATION_RESOLVE_MSG_ACK_RECEIVED`` (only when ``request_ack`` is set), and ``SID_LOCATION_RESOLVE_MSG_TIMEOUT`` statuses.

.. note::
   Since the Amazon Sidewalk SDK v2.0, the ``resolve_location`` field is a bit mask of the ``sid_msg_desc_tx_location_flags`` values instead of a boolean.
   Use ``SID_TX_LOCATION_LINK_RESOLVE`` instead of ``true``.

For details, see the location resolve section of the `Amazon Sidewalk Location Library Developer Guide`_.

.. _location_services_troubleshooting:

Troubleshooting
***************

If you run into problems, the following list includes common issues and their solutions:

* ``No positioning data in the AWS IoT Core`` - Ensure that positioning is activated for your device in the AWS IoT Core console, and a destination is configured.
* ``Sidewalk not ready`` - Make sure Sidewalk is initialized and started before initializing location services.
* ``Initialization order`` - Always initialize location services after calling ``sid_init()`` and deinitialize before calling ``sid_deinit()``.
* ``BLE location not working`` - Ensure the device is connected to a Sidewalk gateway over Bluetooth LE, and the gateway has location services enabled.
* ``FSK gateway location not working`` - Ensure that the FSK link is up (``sid last_status``) and that ``SID_LOCATION_METHOD_FSK_GATEWAY`` is enabled in the location configuration.
* ``LoRa multilateration not resolved`` - Ensure that ``SID_LOCATION_METHOD_LORA_MLAT`` is enabled and ``max_effort`` is at least ``SID_LOCATION_EFFORT_L5``.
  The uplink must be received by at least three Sidewalk gateways, so test it in an area with dense LoRa gateway coverage.

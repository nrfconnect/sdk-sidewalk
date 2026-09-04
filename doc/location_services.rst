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

Supported Location Methods
===========================

The location library supports three types of location resolution methods:

* Sidewalk Network Location over Bluetooth LE - Uses existing Bluetooth LE connections to determine location through the Sidewalk network.
* Wi-Fi Scan - Scans for nearby Wi-Fi access points and sends MAC addresses to the cloud for location resolution.
* GNSS Scan - Uses Global Navigation Satellite System to collect satellite vehicle data for coordinate resolution.

These methods are supported on two available link types:

* Bluetooth LE
* LoRa (Long Range)

.. note::
   FSK with Wi-Fi and GNSS scanning enabled is not currently supported.

GNSS scanning uses the Semtech LoRa Basics Modem middleware to manage and perform scans on the device, and requires the LR1110 radio.
Wi-Fi scanning can use either the LR1110 with the same middleware, or an nRF70 Series Wi-Fi companion IC.
The two implementations are mutually exclusive, so only one of them can be enabled in a build.
For more information about the nRF70 Series-based Wi-Fi location, see :ref:`location_services_nrf70`.

.. list-table:: Hardware and Library Requirements for Location Methods
   :header-rows: 1

   * - **Location Method**
     - **Required Hardware**
     - **Supported Sidewalk Library Variants**
   * - BLE Location
     - nRF chip
     - BLE only, or LoRa/FSK
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
   However, you can still use Bluetooth LE location features while using the SX1262 for LoRa and FSK communication.
   To add Wi-Fi-based location, use an nRF70 Series Wi-Fi companion IC.

Location Levels
===============

Location levels are arranged from lowest to highest effort, based on power usage and time required.
The available hardware on the device determines which levels can be used.
If a higher location level fails or times out, the library automatically moves to the next level after a set timeout.

For more details on integrating device location services with Amazon Sidewalk, refer to the AWS post `Introducing AWS IoT Core Device Location Integration with Amazon Sidewalk`_

This article provides guidance on enabling Device Location for Sidewalk devices, configuring geolocation options in AWS IoT Core, and understanding how location data is transmitted and resolved within the AWS ecosystem.

Level 1: Connected to Sidewalk through BLE
------------------------------------------

At this level, the device uses its existing Sidewalk Bluetooth LE connection to resolve location.

* Power Consumption - No additional power used
* Description - If the device can be located through Bluetooth LE over the Sidewalk network, the device notifies the cloud to resolve the location without extra effort.
  If the device is not connected through Bluetooth LE or a location cannot be resolved, the library moves to the next level.
  The device’s location is based on the Amazon Hub it is connected to.

Level 2: Reserved
-----------------

This level is not yet supported and will default to the next level if reached.

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

.. _location_services_nrf70:

Wi-Fi location using the nRF70 Series
*************************************

As an alternative to the LR1110-based Wi-Fi scan, Wi-Fi location can be resolved using an nRF70 Series Wi-Fi companion IC.
The nRF70 Series does not provide GNSS, so the GNSS location level remains unavailable in such a build.

* Support is enabled automatically through the ``CONFIG_SIDEWALK_NRF7X_LOCATION`` Kconfig option when building with an nRF7002 EB II shield.
  See :ref:`sid_end_device_wifi_location_hw` for the supported shield variants.
* The nRF70 Series-based location is mutually exclusive with the LR1110-based location.
  It is unavailable when the LR1110 is selected as the sub-GHz radio, which is also the case for the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`.
* The companion IC runs in the scan-only mode of the nRF70 driver (the ``CONFIG_NRF70_SCAN_ONLY`` Kconfig option, enabled by default for this feature), which provides Wi-Fi scanning without Wi-Fi connectivity.
* The Wi-Fi scan type can be selected with the ``SIDEWALK_NRF7X_LOCATION_SCAN_TYPE`` Kconfig choice:

  * ``CONFIG_SIDEWALK_NRF7X_LOCATION_SCAN_ACTIVE`` (default) - Resolves access points faster, at the cost of higher power consumption.
  * ``CONFIG_SIDEWALK_NRF7X_LOCATION_SCAN_PASSIVE`` - Lower power consumption, but slower access point discovery.

Testing
*******

The Sidewalk location feature is implemented in the DUT application.
For testing instructions, see :ref:`variant_sidewalk_dut_test_location`.

Sidewalk libraries
==================

Location service is supported in Sidewalk libraries in the following range:

* Sidewalk Sub-GHz library (LoRa and FSK) supports all location methods.
  However to build with radio and pal componets for WiFi and GNSS scanning, ``CONFIG_SIDEWALK_SUBGHZ_RADIO_LR1110`` must be enabled.
  This config is enabled automatically when sample is build with ``semtech_lr11xxmb1xxs`` shield.
  Alternatively, the Wi-Fi scanning alone can be provided by an nRF70 Series companion IC, as described in :ref:`location_services_nrf70`.

* Sidewalk Bluetooth LE only library supports the network location method over Bluetooth LE, and the Wi-Fi location method when an nRF70 Series companion IC is used.

Writing custom application
**************************

For detailed API documentation and usage guide for developers, refer to the `Amazon Sidewalk Location Library Developer Guide`_.

.. _location_services_troubleshooting:

Troubleshooting
***************

If you run into problems, the following list includes common issues and their solutions:

* ``No positioning data in the AWS IoT Core`` - Ensure that positioning is activated for your device in the AWS IoT Core console, and a destination is configured.
* ``Sidewalk not ready`` - Make sure Sidewalk is initialized and started before initializing location services.
* ``Initialization order`` - Always initialize location services after calling ``sid_init()`` and deinitialize before calling ``sid_deinit()``.
* ``BLE location not working`` - Ensure the device is connected to a Sidewalk gateway over Bluetooth LE, and the gateway has location services enabled.

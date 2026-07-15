.. _sidewalk_configuration:

Sidewalk configuration
######################

.. contents::
   :local:
   :depth: 2

This page describes the most important Kconfig options for configuring Sidewalk features.
For sample-specific options, such as variant selection and automatic start behavior, see :ref:`sidewalk_end_device`.

Core protocol support
*********************

The following Kconfig options enable the Sidewalk stack and related product features such as firmware update, manufacturing data support, and on-device certification.

* ``CONFIG_SIDEWALK`` - Enables Amazon Sidewalk support.

* ``CONFIG_SIDEWALK_DFU`` - Enables the Sidewalk Device Firmware Update (DFU) support.

* ``CONFIG_SIDEWALK_DFU_SERVICE_BLE`` - Enables the Zephyr SMP service over Bluetooth LE for new firmware image upload.

* ``CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7`` - Enables support for Sidewalk manufacturing HEX formats version 7 and earlier.
  Use this option only when the application needs to consume legacy manufacturing data format.

* ``CONFIG_SIDEWALK_ON_DEV_CERT`` - Enables the on-device certification shell.

Physical link selection
***********************

Sidewalk end devices can connect over Bluetooth LE only, or over Bluetooth LE together with sub-GHz links (LoRa and FSK).
Use the following option to include link types in the build.

* ``CONFIG_SIDEWALK_SUBGHZ_SUPPORT`` - Enables Sidewalk libraries with Bluetooth LE, LoRa, and FSK support.
  Disabling this option results in using Sidewalk libraries with Bluetooth LE support only.
  While this results in a smaller memory footprint for the application, it also limits its functionality, as connectivity over LoRa or FSK is not available.

  .. note::
     This option is enabled by default when a LoRa transceiver is enabled with the devicetree chosen property ``zephyr,lora-transceiver``.

Secure storage
**************

Persistent Sidewalk keys can be stored in PSA trusted storage instead of the manufacturing partition.
Use the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE`` Kconfig option to enable the feature. 
For storage backend selection and recommended settings, see :ref:`secure_storage`.

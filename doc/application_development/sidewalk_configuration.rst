.. _sidewalk_configuration:

Sidewalk configuration
######################

This page describes the most important Kconfig options for configuring Sidewalk features.
Sample-specific options, such as application variant selection and automatic start behavior, are documented in the :ref:`sidewalk_end_device` sample variants.

Core protocol support
*********************

* ``CONFIG_SIDEWALK`` - Enables Amazon Sidewalk support.

* ``CONFIG_SIDEWALK_DFU`` - Enables the Sidewalk Device Firmware Update (DFU) support.

* ``CONFIG_SIDEWALK_DFU_SERVICE_BLE`` - Enables the Zephyr SMP service over Bluetooth LE for new firmware image upload.

* ``CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7`` - Enables support for Sidewalk manufacturing HEX formats version 7 and earlier.
  Use this option only when the application needs to consume legacy manufacturing data format.

* ``CONFIG_SIDEWALK_ON_DEV_CERT`` - Enables the on-device certification shell.

Physical link selection
***********************

* ``CONFIG_SIDEWALK_SUBGHZ_SUPPORT`` - Enables Sidewalk libraries with Bluetooth LE, LoRa, and FSK support.
  Disabling this option results in using Sidewalk libraries with Bluetooth LE support only.
  While this results in a smaller memory footprint for the application, it also limits its functionality, as connectivity over LoRa or FSK is not available.
  
  .. note::
     This option is enabled by default when an enabled LoRa trasceiver is enabled with the devicetree chosen property ``zephyr,lora-transceiver``.

Secure storage
**************

Use ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE`` to enable secure storage for persistent Sidewalk keys.
For recommended persistent key storage settings, see :ref:`secure_storage`.

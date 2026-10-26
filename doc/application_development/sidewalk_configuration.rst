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

Prebuilt library variant
************************

By default, the application links prebuilt Sidewalk libraries, which are available in a release and a debug variant.
Use the following Kconfig options to configure which variant of these libraries is used.
The variant selection is independent of the link type selection described in `Physical link selection`_, so you can combine either variant with Bluetooth LE-only or sub-GHz builds.

* ``CONFIG_SIDEWALK_USE_PREBUILT_LIBRARIES`` - Uses the prebuilt Sidewalk libraries.
  This option is enabled by default.
  The following variant options are available only when this option is enabled:

  * ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` - Uses the release variant of the Sidewalk libraries (default).
    The release libraries are optimized for size and do not include debug logs.

  * ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` - Uses the debug variant of the Sidewalk libraries.
    The debug libraries include debug logs and additional diagnostics, which results in a significantly larger memory footprint.

For example, to build with the debug libraries, add the following option to the build command:

.. code-block:: console

   west build -b nrf54l15dk/nrf54l15/cpuapp -- -DCONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG=y

Secure storage
**************

Persistent Sidewalk keys can be stored in PSA trusted storage instead of the manufacturing partition.
Use the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE`` Kconfig option to enable the feature.
For storage backend selection and recommended settings, see :ref:`secure_storage`.

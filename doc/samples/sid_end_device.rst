.. _sidewalk_end_device:

Sidewalk end device
###################

.. contents::
   :local:
   :depth: 2

This sample demonstrates how to implement an end device for the Sidewalk End Device application.
It supports Bluetooth® LE, LoRa, and FSK link modes.
It also uses external Quad Serial Peripheral Interface (QSPI) Flash for firmware updates.

.. _sid_requirements:

Requirements
************

This sample supports the following development kits:

+--------------------+----------+-------------------+----------------------------------+
| Hardware platforms | PCA      | Board name        | Build target                     |
+====================+==========+===================+==================================+
| nRF52840 DK        | PCA10056 | `nrf52840dk`_     | ``nrf52840dk/nrf52840``          |
+--------------------+----------+-------------------+----------------------------------+
| nRF5340 DK         | PCA10095 | `nrf5340dk`_      | ``nrf5340dk/nrf5340/cpuapp``     |
+--------------------+----------+-------------------+----------------------------------+
| Thingy53           | PCA20053 | `thingy53`_       | ``thingy53/nrf5340/cpuapp``      |
+--------------------+----------+-------------------+----------------------------------+
| nRF54L15 PDK  `*`  | PCA10156 | `nrf54l15pdk`_    | ``nrf54l15pdk/nrf54l15/cpuapp``  |
+--------------------+----------+-------------------+----------------------------------+

`*` The `nRF54L15 PDK`_ does not support LoRa and FSK configurations.

To run the sample in the Bluetooth LE link mode, you only need the development kit.
However, if you want to run the sample with LoRa or FSK configuration, you also need the LoRa radio module.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. note::
   To use sub-GHz radio, the Semtech shield must be connected to the development kit header, and the antenna must be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.

Overview
********

The purpose of the Sidewalk end device sample is to use it as a starting point when implementing a Sidewalk device.
The sample demonstrates a simple Sidewalk application that allows you to send and receive messages, as well as update firmware.
It also has a few variants you can choose from depending on your application needs.

Source file setup
*****************

The application consists of two source files:

* :file:`main.c` -- The main source file. Starts Sidewalk application.
* :file:`sidewalk.c` (:file:`sidewalk.h`) -- Sidewalk thread file.
  It is a common source file for all sample configurations.
  It implements a separate thread that calls the Sidewalk API.
  The file also includes support for the device firmware update (DFU).
  The following diagram shows the behavior of the state machine:

   .. figure:: ../images/sidewalk_thread_state_machine.svg
      :alt: Sidewalk thread state machine

      Sidewalk thread state machine.

Additionally, it has a variant-specific file:

* :file:`app.c` (:file:`app.h`) -- A file that implements the application's logic.
  It communicates to the Sidewalk thread using events defined in the :file:`sidewalk.h` file, and receives Sidewalk callbacks defined in the :file:`sid_api.h` file.
  An application may consist of multiple ``app_*`` files that are used to define user interface and realize application's logic.

.. _sidewalk_end_device_configuration:

Configuration overview
**********************

When running the sample, you can use different configuration files depending on the supported features.

Configuration files
===================

Configuration files are provided for different build types and are located in the application's root directory.
Build types are covered by dedicated files with the respective build type added to the file name.
The same naming convention is followed if a board has other configuration files that are, for example, associated with a partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
The sample supports the following build types:

.. list-table::
   :widths: auto
   :header-rows: 1

   * - Build type
     - File name
     - Supported board
     - Description
   * - Debug (default)
     - :file:`prj.conf`
     - All from `Requirements`_
     - Debug version of the application.
       It can be used to enable additional features for verifying the application behavior, such as logs or command-line shell.
   * - Release
     - :file:`prj_release.conf`
     - All from `Requirements`_
     - Release version of the application.
       It can be used to enable only the necessary application functionalities to optimize its performance.

You can build the ``release`` firmware for ``build_target`` by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DFILE_SUFFIX=release

For example:

.. code-block:: console

   $ west build -b nrf52840dk/nrf52840 -- -DFILE_SUFFIX=release

.. note::
    Selecting a build type is optional.
    However, if the build type is not selected, the ``debug`` build type is used by default.

Secure Key Storage support
==========================

The Sidewalk Platform Abstraction Layer for nRF Connect SDK supports a trusted storage mechanism for non-volatile Sidewalk keys, ensuring they are stored persistently and confidentially.
The feature is enabled by default with the ``CONFIG_SIDEWALK_CRYPTO_PSA_KEY_STORAGE`` Kconfig option.

.. note::

   Once keys are moved to trusted storage, it is impossible to retrieve them back into MFG storage.
   Therefore, you must not update your firmware to versions that do not support trusted storage for non-volatile Sidewalk keys.

Device Firmware Upgrade support
===============================

The sample supports over-the-air (OTA) device firmware upgrade (DFU) using one of the two following protocols:

* Simple Management Protocol (SMP) over Bluetooth® LE.
  In this case, the DFU is performed using a smartphone application or a PC command line tool.
  This protocol is not a part of the Sidewalk specification.
* Downlink file transfer over Bluetooth LE (experimental).
  This method uses the Sidewalk Bulk Data Transfer (SBDT) mode and integration of AWS IoT FUOTA service to send files to fleet of IoT devices from the AWS IoT FUOTA task.
  For more details, see the Sidewalk Bulk Data Transfer section in the `Amazon Sidewalk specification`_.

   .. note::

      To test this functionality, you must have access to S3 bucket.
      To gain it, contact your account manager or sidewalk-beta-support@amazon.com.
      In the message provide your AWS account ID to be added to the allowlist.

In both cases, the MCUboot secure bootloader is used to apply the new firmware image.

The DFU over SMP is enabled by default.

The following configuration arguments are available during the build process for configuring DFU:

* To enable support for the downlink file transfer over Bluetooth LE, use the ``-DCONFIG_SIDEWALK_FILE_TRANSFER=y`` build flag.

Selecting a sample variant
**************************

You can choose a sample variant depending on your needs.
Testing application scenarios differ depending on the selected Sidewalk configuration:

* ``overlay-hello`` -- The default configuration.
  Use it to run the Hello Sidewalk variant.
* ``overlay-demo`` -- Use this configuration to run the Sensor monitoring variant.
* ``overlay-dut`` -- Use this configuration to run the Device under test (DUT) variant.
  The overlay enables CLI and disables sample automation.

You can build an end device application with the selected overlay by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DOVERLAY_CONFIG="*overlay-name*.conf"

For example, to build the Sidewalk DUT configuration overlay, run the following command:

.. code-block:: console

   $ west build -b nrf52840dk/nrf52840 -- -DOVERLAY_CONFIG="overlay-dut.conf"

See the details on running and testing individual variants, as well as enabling available Kconfig options:

.. toctree::
   :maxdepth: 1
   :glob:

   variants/hello.rst
   variants/sensor_monitoring.rst
   variants/dut.rst

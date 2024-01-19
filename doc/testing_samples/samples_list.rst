.. _samples_list:

Sidewalk Template
#################

This sample demonstrates a template for the Sidewalk End Node application.
It supports Bluetooth LE, LoRa, and FSK link modes.
External QSPI Flash is used for firmware updates.

Requirements
************

This sample supports the following development kits:

+--------------------+----------+------------------------+------------------------------+
| Hardware platforms | PCA      | Board name             | Build target                 |
+====================+==========+========================+==============================+
| nRF52840 DK        | PCA10056 | `nrf52840dk_nrf52840`_ | ``nrf52840dk_nrf52840``      |
+--------------------+----------+------------------------+------------------------------+
| nRF5340 DK         | PCA10095 | `nrf5340dk_nrf5340`_   | ``nrf5340dk_nrf5340_cpuapp`` |
+--------------------+----------+------------------------+------------------------------+

To run the sample in the Bluetooth LE link mode, you only need the development kit.
However, if you want to run the sample with LoRa or FSK configuration, you also need the LoRa radio module.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. note::
   The LoRa radio module shield must be connected to the development kit header, and the antenna must be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.


Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The Sidewalk Template demonstrates a simple Sidewalk application that allows you to send and receive messages, as well as update firmware.

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The following table demonstrates configuration for the supported development kits and differences between samples:

.. tabs::

   .. tab:: nRF52840

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader (in release and debug configs)     | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sidewalk_template`    | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | --                            | Supported             | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

   .. tab:: nRF5340

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader (in release and debug configs)     | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sidewalk_template`    | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | --                            | Supported             | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

Configuration
*************

When running the sample, you can use different configuration files depending on the supported features.

.. _sidewalk_template_build_type:

Configuration files
===================

Configuration files are provided for different build types and are located in the application's root directory.

The :file:`prj.conf` file represents the ``debug`` build type.
Other build types are covered by dedicated files with the respective build type added to the file name.
For example, the file for the ``release`` build type is named :file:`prj_release.conf`.
The same naming convention is followed if a board has other configuration files that are, for example, associated with the partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
Depending on the selected board, a sample supports the following build types:

* ``debug`` -- Debug version of the application.
  It can be used to enable additional features, such as logs or command-line shell, to verify the application behavior.
* ``release`` -- Release version of the application.
  It can be used to enable only the necessary application functionalities to optimize its performance.
* ``no_dfu`` -- No DFU version of the application.
  It can be used in simple scenarios where DFU is not required.
  Additionally, it can be treated as a starting point when integrating the Sidewalk sample with a custom bootloader.

You can build the ``release`` firmware for ``build_target`` by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DCONF_FILE=prj_release.conf

For example:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf

.. note::
    Selecting a build type is optional.
    However, if the build type is not selected, the ``debug`` build type is used by default.

.. _sidewalk_template_config_options:

Configuration options for Sidewalk
==================================

* ``CONFIG_SIDEWALK`` -- Enables support for the Sidewalk protocol and its dependencies.

* ``CONFIG_SIDEWALK_DFU`` -- Enables the nRF Connect SDK DFU service over Bluetooth LE.

* ``CONFIG_SIDEWALK_LINK_MASK`` -- Sets the Sidewalk link mask that the application starts with. The available options are:

	* ``CONFIG_SIDEWALK_LINK_MASK_BLE`` -- Allows connection over Bluetooth LE. It provides maximal throughput but has minimal range. This Configuration is a default choice.
	* ``CONFIG_SIDEWALK_LINK_MASK_FSK`` --  Allows connection over sub-GHz radio using Frequency-Shift Keying. It offers medium throughput and medium range.
	* ``CONFIG_SIDEWALK_LINK_MASK_LORA`` -- Allows connection over sub-GHz radio using the LoRa protocol. It has minimal throughput but offers maximal range.

.. _sidewalk_template_building_and_running:

Building and Running
********************

This sample can be found under :file:`samples/template`.

.. note::
   Before you flash your Sidewalk sample, make sure you have completed the following:

      * You downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * You provisioned your device during the :ref:`setting_up_sidewalk_product`.

   This step needs to be completed only once.
   You do not have to repeat it for every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type instead of a default one, see :ref:`sensor_monitoring_selecting_build`.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Testing:

   sidewalk_application_samples/sensor_monitoring.rst
   sidewalk_application_samples/template.rst
   sidewalk_application_samples/dut.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

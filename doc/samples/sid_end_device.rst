.. _sidewalk_end_device:

Sidewalk end device
###################

This sample demonstrates how to implement an end device for the Sidewalk End Device application.
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
   To use sub-GHz radio, the Semtech shield must be connected to the development kit header, and the antenna must be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.

Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The Sidewalk end device demonstrates a simple Sidewalk application that allows you to send and receive messages, as well as update firmware.

Sample variants
***************

Testing application scenarios are different dependent of the Sidewalk configuration was chosen.

.. toctree::
   :maxdepth: 1
   :glob:

   variants/hello.rst
   variants/demo.rst
   variants/dut.rst

.. _sidewalk_end_device_configuration:

Configuration
*************

When running the sample, you can use different configuration files depending on the supported features.

Configuration overlays
======================

* ``overlay-hello`` -- Configuration for the Hello Sidewalk variant.
  It is also a default configuration. 
* ``overlay-demo`` -- Configuration for the Sensor monitoring variant.
* ``overlay-dut`` -- Configuration for the Device under test (DUT) variant.
  The overlay enables CLI and disables sample automation.

You can build the end device application with the Sidewalk DUT configuration overlay by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DOVERLAY_CONFIG="overlay-dut.conf"

For example:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="overlay-dut.conf"



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

.. _sidewalk_end_device_config_options:

Configuration options for Sidewalk
==================================

* ``CONFIG_SIDEWALK`` -- Enables support for the Sidewalk protocol and its dependencies.

* ``CONFIG_SIDEWALK_SUBGHZ_SUPPORT`` -- Enables use of Sidewalk libraries with Bluetooth LE support only. While this results in a smaller memory footprint for the application, it also limits the functionality (connectivity over LoRa or FSK is not available).

* ``CONFIG_SIDEWALK_DFU`` -- Enables the nRF Connect SDK bootloader and DFU service over Bluetooth LE.

* ``CONFIG_SID_END_DEVICE`` -- Switches between the application variants.

   * ``CONFIG_SID_END_DEVICE_HELLO`` -- Enables the Hello Sidewalk application. 
     This is the default option. 
     For more details, see the :ref:`sidewalk_hello` page.
   * ``CONFIG_SID_END_DEVICE_SENSOR_MONITORING`` -- Enables the Sidewalk Sensor monitoring application. 
     For more details, see the :ref:`sidewalk_demo` page.
   * ``CONFIG_SID_END_DEVICE_DUT`` -- Enables the Sidewalk device under test application. 
     For more details, see the :ref:`sidewalk_dut` page.

* ``CONFIG_SID_END_DEVICE_CLI`` -- Enables Sidewalk CLI. 
  To see the list of available commands, flash the sample and type ``sid help``.

* ``CONFIG_SID_END_DEVICE_AUTO_START`` -- Enables an automatic Sidewalk initialization and start.

* ``CONFIG_SID_END_DEVICE_AUTO_CONN_REQ`` -- Enables an automatic connection request before sending a message. 
  If needed, the Bluetooth LE connection request is sent automatically.

You can build the ``sensor_monitoring`` end device application with Bluetooth LE only libraries by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DCONFIG_SID_END_DEVICE_SENSOR_MONITORING=y -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

For example:

.. code-block:: console

   $ west build -b nrf5340dk_nrf5340_cpuapp -- -DCONFIG_SID_END_DEVICE_SENSOR_MONITORING=y -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

Source file setup
*****************

The application consists of two source files:

* :file:`main.c` -- The main source file. Starts Sidewalk application.
* :file:`app.c` (:file:`app.h`) -- The application specific file.
  It communicates to the sidewalk thread using events defined in :file:`sidewalk.h`, and receives Sidewalk callbacks defined in :file:`sid_api.h`.
  An application may consist of multiple `app_*` files, to define user interface and realize application logic.
* :file:`sidewalk.c` (:file:`sidewalk.h`) -- Sidewalk thread file.
  This is a common source file for all sample configuration.
  It implements a separate thread where the Sidewalk API is called.
  The file also includes the support for the DFU state.
  See the behavior of the state machine outlined in the following diagram:

   .. uml::
      :caption: Sidewalk thread state machine

      [*] --> Sidewalk
      Sidewalk --> [*] : event_factory_reset
      Sidewalk --> FirmwareUpdate : event_dfu
      FirmwareUpdate --> Sidewalk : event_dfu

.. _registering_sidewalk:

Registering Sidewalk Endpoint
*****************************

You can complete Sidewalk registration in one of the two ways:

 * Through an automatic touchless mode that uses Sidewalk Frustration-Free Networking (FFN) where no user interaction is required.

   .. note::
      There are advantages of using automatic FFN registration:

       * There is no association between a user and the Sidewalk Endpoint.
         The Endpoint is allowed to connect to network without being associated to any user.
       * No Login with Amazon (LWA) is needed.

 * Through a manual process by using a local machine to run a registration script and to connect to the Endpoint over Bluetooth LE.

For the exact instructions on both of these methods, refer to the `Registering and testing your Sidewalk endpoint`_ documentation.

.. _Registering and testing your Sidewalk endpoint: https://docs.sidewalk.amazon/provisioning/iot-sidewalk-register-endpoint.html


.. include:: ../ncs_links.rst
.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

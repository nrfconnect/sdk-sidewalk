.. _sidewalk_template:

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
   To use sub-GHz radio, the Semtech shield must be connected to the development kit header, and the antenna must be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.

Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The Sidewalk Template demonstrates a simple Sidewalk application that allows you to send and receive messages, as well as update firmware.

.. _sidewalk_template_user_interface:

User Interface
**************

Common button actions for all template configurations:

Button 1 (short press):
   Sends a message to the cloud. The type of message depends on sample configuration.

Button 1 (long press):
   Enter DFU state - This action disables the Sidewalk stack and starts the Bluetooth LE SMP Server.
   You can update the firmware image using the nRF Connect for mobile application.
   To exit the DFU state, long press **Button 1**.

Button 2 (long press):
   Factory reset - The application informs the Sidewalk stack about the factory reset event.
   The Sidewalk library clears its configuration from the non-volatile storage.
   After a successful reset, the device needs to be registered with the cloud services again.

Button 3 (long press):
   Toggles the Sidewalk link mask - This action switches from Bluetooth LE to FSK, from FSK to LoRa, and from LoRa to Bluetooth LE.
   A log message informs about the link mask switch and the status of the operation.

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

* ``CONFIG_SIDEWALK_SUBGHZ_SUPPORT`` -- Enables use of Sidewalk libraries with Bluetooth LE support only. While this results in a smaller memory footprint for the application, it also limits the functionality (connectivity over LoRa or FSK is not available).

* ``CONFIG_SIDEWALK_DFU`` -- Enables the nRF Connect SDK bootloader and DFU service over Bluetooth LE.

* ``CONFIG_TEMPLATE_APP`` -- Switch between different application types.

   * ``CONFIG_TEMPLATE_APP_HELLO`` -- Enables simple Sidewalk hello world application. This is default option. See :ref:`sidewalk_hello` for more information.
   * ``CONFIG_TEMPLATE_APP_SENSOR_MONITORING`` -- Enables Sidewalk Sensor monitoring application. See :ref:`sensor_monitoring` for more information.

* ``CONFIG_TEMPLATE_APP_CLI`` -- Enables Sidewalk CLI. To see the list of available commands, flash sample and type ``sid help``.

* ``CONFIG_SIDEWALK_AUTO_START`` -- Enable automatic Sidewalk initialization and start.

* ``CONFIG_SIDEWALK_AUTO_CONN_REQ`` -- Enable automatic connection request before message send. The Bluetooth LE connection request action is performed automatically if needed.

You can build the ``sensor_monitoring`` template application, with Bluetooth LE only libraries for ``build_target`` by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DCONFIG_TEMPLATE_APP_SENSOR_MONITORING=y -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

For example:

.. code-block:: console

   $ west build -b nrf5340dk_nrf5340_cpuapp -- -DCONFIG_TEMPLATE_APP_SENSOR_MONITORING=y -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=n

Configuration overlays
======================

* ``overlay-dut`` -- Sidewalk Device Under Test configuration. Enables CLI, disables sample automation.

You can build the template application with Sidewalk DUT configuration overlay for ``build_target`` by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DOVERLAY_CONFIG="overlay-dut.conf"

For example:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="overlay-dut.conf"


.. _sidewalk_template_source_files:

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

.. _sidewalk_template_testing:

Testing
*******

Testing application scenarios are different dependent of the Sidewalk configuration was chosen.

.. toctree::
   :maxdepth: 1
   :glob:

   template_testing/hello_sidewalk.rst
   template_testing/sensor_monitoring.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

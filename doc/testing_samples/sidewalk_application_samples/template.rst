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

In addition to the development kit, a LoRa radio module is also required for this sample if you would like to use the LoRa or FSK configuration.
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
The Sidewalk status is indicated by LEDs state and is also printed in the device logs.
It supports actions, such as sending messages, performing factory reset, and entering the DFU state.
The sample echoes received Sidewalk messages of type GET and SET to the Sidewalk cloud.

User Interface
**************

Button 1 (short press):
   Send Hello -  This action queues a message to the cloud.
   If Sidewalk is not ready, it displays an error without sending the message.
   On Bluetooth LE, the application performs Connection Requests before sending.

Button 1 (long press):
   Enter DFU state - This action disables the Sidewalk stack and starts the Bluetooth LE SMP Server.
   You can update the firmware image using the nRF Connect for mobile application.
   To exit the DFU state, long press **Button 1**.

Button 2 (short press):
   Set Connection Request - The device requests the Sidewalk Gateway to initiate a connection while the device advertises through Bluetooth LE.
   Gateways may not process this request, as it depends on the number of devices connected to it.

Button 2 (long press):
   Factory reset - The application informs the Sidewalk stack about the factory reset event.
   The Sidewalk library clears its configuration from the non-volatile storage.
   After a successful reset, the device needs to be registered with the cloud services again.

Button 3:
   Toggles the Sidewalk link mask - This action switches from Bluetooth LE to FSK, from FSK to LoRa, and from LoRa to Bluetooth LE.
   A log message informs about the link mask switch and the status of the operation.

LED 1:
   Application established a link and connected successfully.
   It is ready to send and receive Sidewalk messages.

LED 2:
   Application time synced successfully.

LED 3:
   Application registered successfully.

LED 4:
   Application woke up successfully.

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

Configuration options
=====================

* ``CONFIG_APP_BLE_ONLY`` -- The configuration enables the use of Sidewalk libraries with Bluetooth LE support only. While this results in a smaller memory footprint for the application, it also limits the functionality (connectivity over LoRa or FSK is disabled).

.. include:: sidewalk_configurations.txt

.. _sidewalk_template_source_files:

Source file setup
=================

The application consists of two source files:

* :file:`main.c` -- The main application file.
  It implements Sidewalk callbacks, assigns button actions, and initializes components.
* :file:`sidewalk.c` -- Sidewalk thread file.
  It implements a separate thread where the Sidewalk API is called to start, stop, process, switch link modes, and send messages.
  The file also includes the support for the DFU state.
  See the behavior of the state machine outlined in the following diagram:

.. uml::
   :caption: Sidewalk thread state machine

   [*] --> Common
   state Common{
   [*] --> Init
   Init --> NotReady : event_not_ready
   NotReady --> Ready : event_ready
   Ready --> NotReady : event_not_ready
   Ready --> Ready : event_send_hello
   }
   Common --> [*] : event_factory_reset
   Common --> Common : event_link_switch
   Common --> FirmwareUpdate : event_dfu
   FirmwareUpdate --> Common : event_dfu

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

.. _sidewalk_template_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

.. include:: testing_sample_include.txt

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

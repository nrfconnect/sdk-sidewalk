.. _sample:

Sidewalk Template
#################

.. contents::
   :local:
   :depth: 2

The sample demonstrates a template for the Sidewalk End Node application.
It supports BLE LoRa and FSK link modes.
The external QSPI Flash is used for firmware updates.


Requirements
************

The sample supports the following development kits:

+--------------------+----------+------------------------+------------------------------+
| Hardware platforms | PCA      | Board name             | Build target                 |
+====================+==========+========================+==============================+
| nRF52840 DK        | PCA10056 | `nrf52840dk_nrf52840`_ | ``nrf52840dk_nrf52840``      |
+--------------------+----------+------------------------+------------------------------+
| nRF5340 DK         | PCA10095 | `nrf5340dk_nrf5340`_   | ``nrf5340dk_nrf5340_cpuapp`` |
+--------------------+----------+------------------------+------------------------------+

Besides the development kit, a LoRa radio module is also needed for this sample if you build it with the LoRa or FSK configuration.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. note::
   The LoRa radio module shield must be connected to the development kit header, and the antenna has to be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.


Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The Template sample supports all available configurations but because the size and location of the bootloader partitions must be consistent, the sample uses external memory to store the secondary partition for the application.
This solution is required only for the Device Firmware Update (DFU).


Wiring*
*******

TODO: semtech wiring

User interface*
***************

Button 1:
   Send Hello - This action will queue a message to the cloud.
   If Sidewalk is not ready, it will simply show an error without sending the message.
   The queue will be processed eventually, and all the queued messages will be sent.
   On BLE application performs Connection Requests before sending automatically if necessary.

Button 2:
   Toggles link mask. BLE/FSK/LoRa

Button 4:
   Factory reset - The application informs the Sidewalk stack about the factory reset event.
   The Sidewalk library clears its configuration from the non-volatile storage.
   After a successful reset, the device needs to be registered with the cloud services again.

LED 1:
   On when not connected. Sidewalk started but not ready to send messages.

LED 2:
   On when connected. Sidewalk is ready to send a message.



Configuration*
**************

When running the sample, you can use different configurations files depending on the supported features.

.. _template_subghz_build_type:

Build types
===========

Configuration files are provided for different build types and are located in the application root directory.

The :file:`prj.conf` file represents a ``debug`` build type.
Other build types are covered by dedicated files with the respective build type added to the file name.
For example, the ``release`` build type file is named :file:`prj_release.conf`.
The same naming convention is followed if a board has other configuration files that are, for example, associated with the partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
Depending on the selected board, a sample supports the following build types:

* ``debug`` -- Debug version of the application - can be used to enable additional features, such as logs or command-line shell, to verify the application behavior.
* ``release`` -- Release version of the application - can be used to enable only the necessary application functionalities to optimize its performance.
* ``no_dfu`` -- No dfu version of the application - can be used in simple scenarios where dfu is not required. Can also be a starting point to integrate sidewalk sample with custom bootloader.

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

Setup*
======

.. note::
   Add information about the initial setup here, for example, that the user must install or enable some library before they can compile this sample, or set up and select a specific backend.
   Most samples do not need this section.

.. _sample_config_options:

Building and running
********************

This sample can be found under :file:`samples/template_subghz`.

.. note::
   Before you flash you Sidewalk sample, make sure you completed the following:

      * You downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * You provisioned your device during the :ref:`setting_up_sidewalk_product`.

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type instead of a default one, see :ref:`sensor_monitoring_selecting_build`

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.
You should see the following output:

TBD

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

.. include:: testing_sample_include.txt


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

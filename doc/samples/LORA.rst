.. _LoRa_sample:

LoRa: Sidewalk
##############

.. contents::
   :local:
   :depth: 2

The sample demonstrates LoRa transport for Sidewalk protocol.

.. _LoRa_requirements:

Requirements
************

The sample supports the following development kit:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

Besides the development kit, LoRa radio module is also needed for this sample.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. _LoRa_overview:

Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The sample shows implementation of Sidewalk API that uses Bluetooth LE for registration and LoRa as a transport protocol.

.. _LoRa_wiring:

Wiring
*******

The LoRa radio module shield must be connected to Arduino header of the development kit (connectors: P13, P14, P15, P16), and the antenna has to be connected to the radio module.

.. _LoRa_ui:

User interface
***************

Refer to the descriptions below:

* Button 1:
   Factory reset.

* Button 2:
   Set device profile - set unicast attributes of the device.

* Button 3:
   Action button - send message to AWS cloud.

* LED 1:
   ``ON`` when connected.

.. _LoRa_setup:

Setup
======

Before building a sample, follow :ref:`setting_up_the_environment` to set up your environment.

.. _LoRa_building_and_running:

Building and running
********************

#. Find the Sidewalk sample folder.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/samples/template

#. Build the example.
   For more details on building with west, see `West building flashing and debugging`_.

   .. code-block:: console

       $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="lora.conf"

   Alternatively, you can use VS Code to add the build configuration:

       .. figure:: /images/vscode_build_LoRa.png

#. Once the build has been completed successfully, proceed with flashing the device as per :ref:`endpoints_config_flashing_binaries`.

.. _LoRa_testing:

Testing
=======

After successfully building the sample, flashing manufacturing data and registering the device, the device can be tested by following the instruction in :ref:`sidewalk_testing`.


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html

.. _LoRa_sample:

LoRa: Sidewalk
##############

.. contents::
   :local:
   :depth: 2

The sample demonstrates LoRa transport for Sidewalk protocol.

Requirements
************

The sample supports the following development kit:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

For this sample beside development board, LoRa radio module is also needed. Supported modules:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+



Overview
********

You can use this sample as a starting point to implement a Sidewalk device.

The sample shows implementation of Sidewalk API that uses BLE for registration and LoRa as a transport protocol.


Wiring
*******

The LoRa radio module shield must be connected to Arduino header of the development board (connectors: P13, P14, P15, P16)
and antenna has to be connected to the radio module.

User interface
***************

Button 1:
   Factory reset

Button 2:
   Set device profile - set unicast attributes of the device

Button 3:
   Action button - send message to AWS cloud

LED 1:
   ``ON`` when connected.


Setup
======

Before building sample please follow section :ref:`setting_up_the_environment` to set up your environment.

Building and running
********************

#. Find Sidewalk sample folder.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/samples/template

#. Build the example.
   For more details about building with west see `West building flashing and debugging`_.

   .. code-block:: console

       $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="LoRa.conf"


Or use VScode:
    .. figure:: /images/vscode_build_LoRa.png

#. Flashing sample and manufacturing data
   When the build has been completed successfully, proceed with the flashing the device as instructed in :ref:`Flashing binaries<endpoints_config_flashing_binaries>`

Testing
=======

After successful build, flashing manufacturing data and registering the device,
the device can be tested by following the instruction in :ref:`sidewalk_testing`


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html

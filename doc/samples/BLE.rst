.. _ble_sample:

BLE: Sidewalk
#############

.. contents::
   :local:
   :depth: 2

The sample demonstrates BLE transport for Sidewalk protocol.

Requirements
************

Supported development kits are listed in table below:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+


Overview
********

The sample shows implementation of Sidewalk API that uses BLE as a transport protocol. 
This example can be use as reference to develop own Sidewalk device.

Wiring
*******

The sample do not require any additional connection to development kit to work properly.


User interface
***************

Button 1:
   Factory reset

Button 2:
   Connection request

Button 3:
   Action button - send message to AWS cloud

Button 4:
   Change advertised battery level (+1 up to 100, and wrap around to 0)

LED 1:
   ``ON`` when connected.

LED 1 - LED 4:
   All LEDs light continuously when manufacturing data is missing (or has not been flashed).
   
   .. note:
      Prepare manufacturing data and flash it to the device. :ref:`instruction<endpoints_config_flashing_certs>`

Setup
======

Before building sample please follow section :ref:`setting_up_the_environment` to set up your environment.

Building and running
********************

For building sample please follow :ref:`endpoints_config_building_sample`, after flashing the device can be register in following way :ref:`reg_associating_sidewalk_endpoints`

Testing
=======

After successful build, flashing manufacturing data and registering the device,
the device can be tested by following the instruction in :ref:`sidewalk_testing`

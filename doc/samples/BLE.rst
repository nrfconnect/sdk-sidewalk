.. _ble_sample:

Bluetooth LE: Sidewalk
######################

.. contents::
   :local:
   :depth: 2

The sample demonstrates Bluetooth LE transport for Sidewalk protocol.

.. _ble_requirements:

Requirements
************

The sample supports the following development kit:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

.. _ble_overview:

Overview
********

The sample shows implementation of Sidewalk API that uses Bluetooth LE as a transport protocol.
This example can be used as reference to develop your own Sidewalk device.

.. _ble_wiring:

Wiring
******

The sample does not require any additional connection to a development kit in order to work properly.

.. _ble_ui:

User interface
***************

Refer to the descriptions below:

* Button 1:
   Factory reset.

* Button 2:
   Connection request.

* Button 3:
   Action button - send message to AWS cloud.

* Button 4:
   Change advertised battery level (+1 up to 100, and wrap around to 0).

* LED 1:
   ``ON`` when connected.

* LED 1 - LED 4:
   All LEDs will light continuously when manufacturing data is either missing or has not been flashed.

.. note::
   To prepare manufacturing data and flash it to the device, refer to :ref:`endpoints_config_flashing_certs`.

.. _ble_setup:

Setup
=====

Before building a sample, follow :ref:`setting_up_the_environment` to set up your environment.

.. _ble_building_and_running:

Building and running
********************

For building a sample, follow :ref:`endpoints_config_building_sample`.
Once you flashed the device, it can be register as per :ref:`reg_associating_sidewalk_endpoints`.

.. _ble_testing:

Testing
=======

After successfully building the sample, flashing manufacturing data and registering the device, the device can be tested by following the instruction in :ref:`sidewalk_testing`.

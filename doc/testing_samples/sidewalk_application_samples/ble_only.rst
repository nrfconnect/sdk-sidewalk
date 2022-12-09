.. _ble_only_sample:

Bluetooth LE only
#################

The sample demonstrates a template for Sidewalk End Node application.

.. _ble_only_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

.. _ble_only_overview:

Overview
********

The sample shows implementation of Sidewalk API for the Bluetooth LE only transport protocol.

.. _ble_only_config:

Configuration
*************

Before building a sample, follow the :ref:`setting_up_the_environment` instructions.

Overlays overview

* :file:`usb_dfu` - This configuration adds a USB device to the application which acts as the target for the firmware update.
  A new image is then taken through the USB and placed in the second partition for bootloader.
  For more details, see the `USB DFU Sample Application`_ documentation.


.. _ble_only_ui:

User interface
**************


Refer to the descriptions below:

   * Button 1:
      Factory reset.

   * Button 2:
      Set connection request.

   * Button 3:
      Action button - send message to AWS cloud.

   * LED 1:
      ``ON`` when connected.

   * LED 1 - LED 4:
      All LEDs will light continuously when manufacturing data is either missing or has not been flashed.


.. _ble_only_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device follow the instruction in :ref:`sidewalk_testing`.

.. _USB DFU Sample Application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/samples/subsys/usb/dfu/README.html

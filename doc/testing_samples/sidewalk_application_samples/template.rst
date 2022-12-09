template
########

The sample demonstrates a template for Sidewalk End Node application.

.. _template_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

Besides the development kit, LoRa radio module is also needed for this sample if the LoRa or FSK configuration is being build.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

The LoRa radio module shield must be connected to Arduino header of the development kit (connectors: P13, P14, P15, P16), and the antenna has to be connected to the radio module.

.. _template_overview:

Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The sample shows implementation of Sidewalk API for the following transport protocols:

* Bluetooth LE only - For registration and communication
* LoRa - Bluetooth LE for registration and LoRa for communication
* FSK - Bluetooth LE for registration and FSK for communication

.. _samples_config:

Configuration
*************

Before building a sample, follow the :ref:`setting_up_the_environment` instructions.

Overlays overview

- :file:`lora.conf` - This configuration enables LoRa transport in the Sidewalk application.
  To enable this overlay you have to connect the Semtech sx1262 shield. This overlay changes the button assignment, check :ref:`samples_ui`.

- :file:`fsk.conf` - This configuration enables FSK transport in the Sidewalk application.
  To enable this overlay you have to connect the Semtech sx1262 shield. This overlay changes the button assignment, check :ref:`samples_ui`.

- :file:`usb_dfu` - This configuration adds a USB device to the application which acts as the target for the firmware update.
  A new image is then taken through the USB and placed in the second partition for bootloader. For more details check `usb_dfu_sample`_.


.. _samples_ui:

User interface
**************

Refer to the descriptions below:

.. tabs::

   .. group-tab:: Default
   
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


   .. group-tab:: LoRa/FSK

      * Button 1:
         Factory reset.

      * Button 2:
         Set device profile - set unicast attributes of the device.

      * Button 3:
         Action button - send message to AWS cloud.

      * LED 1:
         ``ON`` when connected.

      * LED 1 - LED 4:
         All LEDs will light continuously when manufacturing data is either missing or has not been flashed.


.. _samples_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device follow the instruction in :ref:`sidewalk_testing`.


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

.. _usb_dfu_sample: https://docs.zephyrproject.org/2.6.0/samples/subsys/usb/dfu/README.html

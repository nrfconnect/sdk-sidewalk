.. _template_ble_sample:

Template BLE
############

The sample demonstrates a template for Sidewalk End Node application.
It is optimized for Bluetooth LE.

.. _template_ble_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

.. _template_ble_overview:

Overview
********

The sample shows implementation of the Sidewalk API for the Bluetooth LE transport protocol.
It is a memory-optimized example of Sidewalk configuration where only Bluetooth LE transport protocol is supported.
Because of the smaller footprint, both bootloader partitions for the application are placed in the internal flash memory of the supported SoC (nRF52840).

.. _template_ble_config:

Configuration
*************

Before building a sample, follow the :ref:`setting_up_the_environment` instructions.

Overlays overview

* :file:`cli` - This configuration adds an nRF Connect Shell over with Sidewalk Command Line Interface.
  For more details, see the :ref:`sidewalk_testing_with_cli` documentation.

* :file:`usb_dfu` - This configuration adds a USB device to the application which acts as the target for the firmware update.
  A new image is then taken through the USB and placed in the second partition for bootloader.
  For more details, see the `USB DFU Sample Application`_ documentation.


.. _template_ble_ui:

User interface
**************


Refer to the descriptions below:

To learn more about actions details see: :ref:`sidewalk_samples_ui`

A button action is triggered when you release the button.
To use a long press action, hold a button for 2 seconds or longer, and release it.

   * Button 1 (long press):
      Factory Reset.

   * Button 2:
      Toggle Connection Request.

   * Button 3:
      Send Hello.

   * Button 4 (short press):
      Set fake battery level.

   * Button 4 (long press):
      Enter DFU state.

LEDs represent the current state of the application:

   * LED 1:
      Application ``Connected`` successfully.

   * LED 2:
      Application ``Registered`` successfully.

   * LED 3:
      Application ``time sync`` successfully.

   * LED 4:
      Application ``link`` is up.

Other states are routed to generic GPIO pins.
When the voltage drops to 0, entering low logic state, it means that one of the following states is active:

   * P1.1:
      ``DFU`` state.

   * P1.2:
      ``ERROR`` has occured.

   * P1.3:
      ``Sending`` pin is active when the packet is being transmitted.

   * P1.4:
      The ``Received`` pin is toggled ``on`` and immediately ``off`` when the packet has been received.

.. _template_ble_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device follow the instruction in :ref:`sidewalk_testing`.

.. _USB DFU Sample Application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/samples/subsys/usb/dfu/README.html

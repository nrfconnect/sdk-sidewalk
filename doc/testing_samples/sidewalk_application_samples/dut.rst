.. _device_under_test_sample:

Device Under Test
#################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

.. _device_under_test_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

If you want to run this sample with LoRa or FSK configuration, you also need the LoRa radio module. 
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. note::
   The LoRa radio module shield must be connected to the development kit header, and the antenna has to be connected to the radio module.

.. _device_under_test_config:

Configuration
*************

Before building the sample, follow the :ref:`setting_up_the_environment` documentation.

.. _device_under_test_ui:

User interface
**************

Device uses UART CLI to communicate with user.

.. _device_under_test_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data, sample is ready to use.
Type :cli:`sid`` to see possible Cli command to use.


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

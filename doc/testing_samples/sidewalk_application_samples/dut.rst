.. _device_under_test:

Device Under Test
#################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

.. _device_under_test_requirements:

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

To run the sample with the Bluetooth configuration, you only need the development kit.
However, if you want to run the sample with LoRa or FSK configuration, you also need the LoRa radio module.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

.. note::
   The LoRa radio module shield must be connected to the development kit header, and the antenna has to be connected to the radio module.
   For the exact pin assignment, refer to the :ref:`setting_up_hardware_semtech_pinout` section.

.. _device_under_test_ui:

User interface
**************

Device uses UART CLI to communicate with the user.
To see the list of available commands, type ``sid help``.

.. _dut_building_and_running:

Building and running
********************

This sample can be found under :file:`samples/sid_dut`.

.. note::
   Before you flash you Sidewalk sample, make sure you completed the following:

      * You downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * You provisioned your device during the :ref:`setting_up_sidewalk_product`.

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.

.. _device_under_test_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
You should see the following output:

   .. code-block:: console

       *** Booting Zephyr OS build v3.2.99-ncs2 ***
       ----------------------------------------------------------------
       sidewalk             v1.14.3-1-g1232aabb
       nrf                  v2.3.0
       zephyr               v3.2.99-ncs2
       ----------------------------------------------------------------
       sidewalk_fork_point = af5d608303eb03465f35e369ef22ad6c02564ac6
       build time          = 2023-03-14 15:00:00.000000+00:00
       ----------------------------------------------------------------
       uart:~$

To start testing the application, you can use the available commands (``sid help``).

.. include:: ../../ncs_links.rst

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

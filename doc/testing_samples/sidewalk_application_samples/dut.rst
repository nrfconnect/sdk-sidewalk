.. _device_under_test:

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

If you want to run this sample with LoRa or FSK configuration, you also need the LoRa Radio Module.
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

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Building and programming an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/programming.html#gs-programming
.. _Testing and debugging an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/testing.html#gs-testing
.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832
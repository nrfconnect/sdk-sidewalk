.. _sensor_monitoring:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IOT use case.
It consists of an edge device (a sensor).
An application server (cloud backend with web UI) compatible with this sample is provided in `Amazon Sidewalk Sample IoT App`_ repository.

.. _sensor_monitoring_requirements:

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

Building and running
********************

This sample can be found under :file:`samples/sensor_monitoring`.

.. note::
   Before you flash you Sidewalk sample, make sure you have:

      * Downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * Provisioned your device during the :ref:`setting_up_sidewalk_product`

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type, see :ref:`sensor_monitoring_selecting_build`.

.. _sensor_monitoring_selecting_build:

Selecting a build type
======================

Use one of the common sample configurations:

* Build with the default configuration (BLE link mode):

   .. code-block:: console

      $ west build -b nrf52840dk_nrf52840

* Build with the FSK link mode:

   .. code-block:: console

      $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="fsk.conf"

* Build with the LoRa link mode:

   .. code-block:: console

      $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="lora.conf"

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.
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

      [00:00:00.006,225] <inf> sid_template: Sidewalk example started!

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

.. note::
   For the full installation guidelines and the application overview, refer to the `Amazon Sidewalk Sample IoT App`_ repository.

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/amazon-sidewalk-sample-iot-app
.. _Building and programming an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/programming.html#gs-programming
.. _Testing and debugging an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started/testing.html#gs-testing
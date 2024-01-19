.. _sensor_monitoring:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IOT use case.
It consists of an edge device (a sensor).
An application server (cloud backend with web UI) compatible with this sample is provided in `Amazon Sidewalk Sample IoT App`_ repository.

Building and running
********************

This sample can be found under :file:`samples/sensor_monitoring`.

.. note::
   Before you flash you Sidewalk sample, make sure you completed the following:

      * You downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * You provisioned your device during the :ref:`setting_up_sidewalk_product`.

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type instead of a default one, see :ref:`sensor_monitoring_selecting_build`.

.. _sensor_monitoring_selecting_build:

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

.. include:: ../../ncs_links.rst

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app

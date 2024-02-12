.. _sensor_monitoring:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IOT use case.
It consists of an edge device (a sensor).
An application server (cloud backend with web UI) compatible with this sample is provided in `Amazon Sidewalk Sample IoT App`_ repository.


User Interface
**************

Button 1-4 (short press):
   Send button action message. This action works after application successfully sends capability message.

LED 1-4:
   Notify LED action message. This action works after application successfully sends capability message.

For common sidewalk template user interface description see ::ref:`sidewalk_template_user_interface`

Configuration
*************

.. note::
    To build Sidewalk template as sensor monitoring application
    the ``CONFIG_TEMPLATE_APP_SENSOR_MONITORING`` must be enabled in sample configuration.

The Sidewalk sensor monitoring demo application supports the following configurations:

* ``CONFIG_TEMPLATE_APP_NOTIFY_DATA_PERIOD_MS`` -- Notify period of sensor monitoring template app in milliseconds.


Testing
*******

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

Wait for the device to complete the automatic registration.

.. note::
   For the full installation guidelines and the application overview, refer to the `Amazon Sidewalk Sample IoT App`_ repository.

.. include:: testing_nordic_dfu_include.txt

.. include:: ../../ncs_links.rst

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app

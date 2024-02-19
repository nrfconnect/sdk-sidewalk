.. _sidewalk_demo:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IoT use case.
It requires an edge device (a sensor).
An application server (cloud backend with web UI) compatible with this sample is provided in `Amazon Sidewalk Sample IoT App`_ repository.


User Interface
**************

.. include:: include_user_interface_common.txt

Button 1-4 (short press):
   Send button action message - This action works after the application successfully sends capability message.

LED 1-4:
   Notify LED action message - This action works after the application successfully sends capability message.

Configuration
*************

.. note::
    To build the Sensor monitoring application variant of the Sidewalk end device, use the ``OVERLAY_CONFIG="overlay-demo.conf"`` configuration.

The Sidewalk sensor monitoring demo application supports the following configurations:

* ``CONFIG_SID_END_DEVICE_NOTIFY_DATA_PERIOD_MS`` -- Enables the notify period of the sensor monitoring end device app in milliseconds.

For more configuration options, see :ref:`sidewalk_end_device_configuration`.

Source file setup
*****************

The application consists of two source files:

* :file:`app.c` (:file:`app.h`) -- The main application file starts the TX and RX thread, assigns button actions to the board's buttons, and adds received messages to the received message queue.
* :file:`app_sensor.c`, :file:`app_button.c`, :file:`app_led.c` -- The files implement interface between the hardware and sample application.
* :file:`app_rx.c` (:file:`app.h`) -- RX thread reads messages form the received messages queue, deserializes them, and triggers actions.
* :file:`app_tx.c` (:file:`app.h`) -- TX thread sends messages from the device to cloud.
  Depending on the actual state of the device, it sends capability messages or sensor data notifications.

  .. uml::
      :caption: Application TX thread state machine

      [*] -> Init
      Init --> Notify_Capacity : time_syc_success
      Notify_Capacity --> Notify_Data : capacity_success
      Notify_Data --> Notify_Capacity : time_syc_fail
      Notify_Data --> Notify_Data : action_response
      Notify_Data --> Notify_Data : action_notify

Building and running
********************

.. include:: include_building_and_running.txt

Testing
*******

.. note::
   For the full installation guidelines and the application overview, refer to the `Amazon Sidewalk Sample IoT App`_ repository.
   Flash the Nordic device with the end device sensor monitoring application instead of the prebuilt HEX file provided in the cloud application repository.

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.
You should see the following output:

   .. code-block:: console

      *** Booting nRF Connect SDK 883c3709f9c8 ***
      ----------------------------------------------------------------
      sidewalk             v2.5.0-3-g1232aabb
      nrf                  v2.5.0-g271e80422
      zephyr               883c3709f9
      ----------------------------------------------------------------
      sidewalk_fork_point = af5d608303eb03465f35e369ef22ad6c02564ac6
      build time          = 2023-03-14 15:00:00.000000+00:00
      board               = nrf52840dk_nrf52840
      ----------------------------------------------------------------
      [00:00:00.001,373] <inf> application_state: working = true
      [00:00:00.055,480] <inf> sidewalk_app: Sidewalk link switch to BLE

Wait for the device to complete the automatic registration.

.. include:: include_testing_nordic_dfu.txt

.. include:: ../../ncs_links.rst
.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app

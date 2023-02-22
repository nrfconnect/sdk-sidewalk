.. _sensor_monitoring_sample:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IOT use case.
It consists of an edge device (a sensor). 
An application server (cloud backend with web UI) compatible with this sample is provided by Amazon.

.. TBD: Link to Amazon server app, when published

.. _sensor_monitoring_requirements:

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

.. _sensor_monitoring_config:

Configuration
*************

Before building the sample, follow the :ref:`setting_up_the_environment` documentation.

.. _sensor_monitoring_ui:

User interface
**************

You can toggle buttons on the edge device which is then reflected through the corresponding button icons in the web UI (uplink communication).  
Similarly, you can toggle LED buttons in the UI view which then triggers the corresponding LEDs in the edge device (downlink communication). 

.. _sensor_monitoring_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device follow the instruction in :ref:`sidewalk_testing`.


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

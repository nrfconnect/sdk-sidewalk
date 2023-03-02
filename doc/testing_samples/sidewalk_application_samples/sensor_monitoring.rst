.. _sensor_monitoring:

Sensor monitoring
#################

This sample demonstrates how a temperature sensor communicates over the secure Sidewalk network, which is a simple Sidewalk-based IOT use case.
It consists of an edge device (a sensor).
An application server (cloud backend with web UI) compatible with this sample is provided by Amazon.

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

.. note::
   For the full installation guidelines and the application overview, refer to the `Amazon Sidewalk Sample IoT App`_ repository.

Building and running
********************

This sample can be found under :file:`samples/sensor_monitoring`.


#. If you are building the sample for the first time, set up the environment:

   .. note::
      This step needs to be completed only once.
      You do not have to repeat it on every sample rebuild.

   a. Run the application manifest and update.

      .. code-block:: console

         $ west config manifest.path sidewalk
         $ west update

   #. Flash the :file:`mfg.hex` file.

      .. code-block:: console

         $ nrfjprog --chiperase --family NRF52 \
         --program ${ZEPHYR_BASE}/../sidewalk/tools/provision/mfg.hex \
         --reset

#. Once you have set up your environment, build the sample.
   Depending on the configuration, build with one of the commands below:

   * Build with the default configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840

   .. note::
      For more details on building with west, see the `West building flashing and debugging`_ documentation.

#. Flash the Sidewalk application.

   .. code-block:: console

	   $ west flash

#. Confirm the Sidewalk sample is flashed.

   a. List a serial port of the connected development kit.

      .. code-block:: console

          $ nrfjprog --com
          683929577    /dev/ttyACM0    VCOM0

   #. Connect to the listed serial port with the following settings:

      * Baud rate: 115200
      * 8 data bits
      * 1 stop bit
      * No parity
      * HW flow control: None

      For example, you can use the Screen tool:

      .. code-block:: console

        $  screen  /dev/ttyACM0 115200

   #. To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.

      .. code-block:: console

         *** Booting Zephyr OS build v3.0.99-ncs1-4913-gf7b06162027d  ***
         ----------------------------------------------------------------
         sidewalk             v1.13.0-6-g2e0691d-dirty
         nrf                  v2.0.0-734-g3904875f6
         zephyr               v3.0.99-ncs1-4913-gf7b0616202-dirty
         ----------------------------------------------------------------
         sidewalk_fork_point = 92dcbff2da68dc6853412de792c06cc6966b8b79
         build time          = 2022-11-17 10:52:50.833532+00:00
         ----------------------------------------------------------------

         [00:00:00.006,225] <inf> sid_template: Sidewalk example started!

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/amazon-sidewalk-sample-iot-app
.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html
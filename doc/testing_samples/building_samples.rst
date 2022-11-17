.. _sidewalk_samples:

Sidewalk application samples
############################

The sample demonstrates a template for Sidewalk End Node application.

.. _samples_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+----------------------+-------------------------+
| Hardware platforms | PCA      | Board name           | Build target            |
+====================+==========+======================+=========================+
| nRF52840 DK        | PCA10056 | nrf52840dk_nrf52840  | ``nrf52840dk_nrf52840`` |
+--------------------+----------+----------------------+-------------------------+

Besides the development kit, LoRa radio module is also needed for this sample.
The supported modules are as follows:

+------------+---------------+------------+
| Radio chip | Board name    | Interface  |
+============+===============+============+
| `SX1262`_  | SX126xMB2xAS  | SPI @ 8Mhz |
+------------+---------------+------------+

The LoRa radio module shield must be connected to Arduino header of the development kit (connectors: P13, P14, P15, P16), and the antenna has to be connected to the radio module.

.. _sample_overview:

Overview
********

You can use this sample as a starting point to implement a Sidewalk device.
The sample shows implementation of Sidewalk API for the following transport protocols:

* Bluetooth LE only - For registration and communication
* LoRa - Bluetooth LE for registration and LoRa for communication

.. _samples_config:

Configuration
*************

Before building a sample, follow the :ref:`setting_up_the_environment` instructions.

.. _samples_building_and_running:

Building and running
********************

If you are building a sample for the first time, see :ref:`samples_first_sample` section.
Otherwise, proceed to :ref:`sample_running`.

.. _samples_first_sample:

Building the first sample
=========================

You need to perform the following steps only once.
You do no have to repeat it on every sample rebuild.

#. Run application manifest and update.

   .. code-block:: console

       $ west config manifest.path sidewalk
       $ west update

#. Flash :file:`mfg.hex` file generated in :ref:`generate_manufacturing_data`.

   .. code-block:: console

       $ nrfjprog --chiperase --family NRF52 \
       --program ${ZEPHYR_BASE}/../sidewalk/tools/provision/mfg.hex \
       --reset

.. _sample_running:

Standard building and running the sample
========================================

#. Build the sample.
   For more details on building with west, see the `West building flashing and debugging`_ documentation.

   a. Bluetooth LE:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840

   #. LoRa:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="lora.conf"

      Alternatively, you can use VS Code to add the build configuration:

         .. figure:: /images/vscode_build_LoRa.png

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

.. _samples_ui:

User interface
**************

Refer to the descriptions below:

* Button 1:
   Factory reset.

* Button 2:
   Set device profile - set unicast attributes of the device.

* Button 3:
   Action button - send message to AWS cloud.

* LED 1:
   ``ON`` when connected.

* LED 1 - LED 4:
   All LEDs will light continuously when manufacturing data is either missing or has not been flashed.

.. _samples_testing:

Testing
=======

After successfully building the sample and flashing manufacturing data wait for the device to complete :ref:`automatic_registration_sidewalk_endpoints`.
To test the device follow the instruction in :ref:`sidewalk_testing`.


.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/
.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html


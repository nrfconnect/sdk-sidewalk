.. _samples_building_and_running:

Building and running
********************

If you are building a sample for the first time, see :ref:`samples_first_sample` section.
Otherwise, proceed to :ref:`sample_running`.

.. _samples_first_sample:

Building the first sample
-------------------------

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
----------------------------------------

#. Build the sample.
   Depending on the configuration, build with one of the commands below:

   * Build the default configuration:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840

   * Build with overlay:

      .. code-block:: console

         $ west build -b nrf52840dk_nrf52840 -- -DOVERLAY_CONFIG="lora.conf"

      Alternatively, you can use VS Code to add the build configuration:

         .. figure:: /images/vscode_build_LoRa.png

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


.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html
.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832

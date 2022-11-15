.. _endpoints_config_building_sample:

Building and flashing sample
############################

Building
********

#. Find the Sidewalk sample folder.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/samples/template

#. Build the example.
   For more details on building with west, see `West building flashing and debugging`_.

   .. code-block:: console

       $ west build -b nrf52840dk_nrf52840

.. _endpoints_config_flashing_binaries:

Flashing binaries
*****************

#. Connect your Nordic device to the Ubuntu machine via USB.
   Set the :guilabel:`Power Key` switch on the device to :guilabel:`ON`.

#. Test the connection between your Ubuntu machine and your nRF52840-DK board by entering the following command in your terminal:

   .. code-block:: console

       $ nrfjprog -i
	   683929577

#. Flash the Sidewalk application.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/samples/template
	   $ west flash --erase

#. Flash :file:`mfg.hex` file generated in :ref:`endpoints_config_flashing_certs`.

   .. code-block:: console

       $ nrfjprog --sectorerase --family NRF52 \
       --program ${NCS_FOR_SIDEWALK_SDK_PATH}/tools/provision/mfg.hex

#. Clear settings partition.
   For a new :file:`mfg.hex` file to take effect, you have to clear application settings and remove any information about the previous configuration.
   You can do this by either running nrfjprog command or performing factory reset:

   .. tabs::

      .. tab:: nrfjprog

         .. code-block:: console

            $ nrfjprog  --erasepage 0xf8000-0xFF000

         .. note::

            By default, settings partition is located between 0xf8000-0xFF000.
            If your settings have a different location, adjust the command accordingly.

      .. tab:: Factory reset

         In all samples by default :guilabel:`Button 1` is responsible for performing factory reset.

         .. note::

            If CLI is enabled (:ref:`Sidewalk CLI <sidewalk_cli>`), it can be used to trigger :guilabel:`Button 1` or to call factory reset directly.

#. Restart the device.

   .. code-block:: console

       $ nrfjprog -r

   Example output:

   .. code-block:: console

       ~$ west flash --erase
		-- west flash: rebuilding
		ninja: no work to do.
		-- west flash: using runner nrfjprog
		-- runners.nrfjprog: mass erase requested
		Using board 683929577
		-- runners.nrfjprog: Flashing file: ncs/sidewalk/samples/template/build/zephyr/zephyr.hex
		Parsing image file.
		Applying system reset.
		Verifying programming.
		Verified OK.
		Enabling pin reset.
		Applying pin reset.
		-- runners.nrfjprog: Board with serial number 683929577 flashed successfully.
       ~$ nrfjprog --sectorerase --family NRF52 --program ${NCS_FOR_SIDEWALK_SDK_PATH}/tools/provision/mfg.hex
       Parsing image file.
       Applying system reset.
       Verified OK.
       ~$ nrfjprog -r
       Applying system reset.
       Run.

#. Confirm the Sidewalk sample is flashed.

   a. List a serial port of the connected DK.

      .. code-block:: console

          $ nrfjprog --com
          683929577    /dev/ttyACM0    VCOM0

   #. Run serial logger (for example the Screen tool).

      .. code-block:: console

        $  screen  /dev/ttyACM0 115200

   #. To see fresh logs, restart the program by pressing the :guilabel:`RESET` button on the DK.

      .. note::

       The sidewalk example started, but the device is not registered.
	    You will see initialization error.

      .. code-block:: console

         *** Booting Zephyr OS build v3.0.99-ncs1-4913-gf7b06162027d  ***
         ----------------------------------------------------------------
         sidewalk             v1.12.1-59-g956ab46-dirty
         nrf                  v2.0.0-734-g3904875f6
         zephyr               v3.0.99-ncs1-4913-gf7b0616202
         ----------------------------------------------------------------
         sidewalk_fork_point = afc57d56a02de561db057b40d07dc2fa9f40adf9
         build time          = 2022-11-14 14:14:23.714266+00:00
         ----------------------------------------------------------------

         [00:00:00.006,195] <inf> sid_template: Sidewalk example started!
         [00:00:00.019,622] <inf> sid_thread: Initializing sidewalk - Link mask BLE
         [00:00:00.020,965] <err> sid_thread: failed to initialize sidewalk, err: -8
         [00:00:00.020,996] <err> sid_thread: resource not found - check if mfg.hex has been flashed


.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html

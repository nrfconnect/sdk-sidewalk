.. _endpoints_config_building_sample:

Building and flashing sample
############################

Building
********

#. Find Sidewalk sample folder.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/samples/template

#. Build the example.
   For more details about building with west see `West building flashing and debugging`_.

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

         *** Booting Zephyr OS build v3.0.99-ncs1  ***
         [00:00:00.004,211] <inf> sid_template: Sidewalk example started!
         [00:00:00.017,517] <inf> fs_nvs: 2 Sectors of 4096 bytes
         [00:00:00.017,547] <inf> fs_nvs: alloc wra: 0, fe8
         [00:00:00.017,578] <inf> fs_nvs: data wra: 0, 0
         [00:00:00.018,310] <err> sidewalk: Sall mgm core create failed: -8
         [00:00:00.018,310] <err> sid_thread: failed to initialize sidewalk, err: -8

.. _West building flashing and debugging: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/develop/west/build-flash-debug.html

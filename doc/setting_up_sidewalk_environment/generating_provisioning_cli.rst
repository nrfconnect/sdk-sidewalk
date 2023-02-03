.. _generate_manufacturing_data_cli:

Generating device manufacturing data
************************************

Before configuring your HDK, ensure you have successfully completed the following steps:

    * :ref:`designing_sidewalk_product` - Where you generated :file:`device_profile.json` and :file:`wireless_device.json` files

In order to flash the keys onto your Sidewalk Endpoint, you have to use the :file:`provision.py` script provided in the Sidewalk software package.

#. Go to :file:`provision` folder.

   .. code-block:: console

       $ cd ${ZEPHYR_BASE}/../sidewalk/tools/provision

#. Copy the :file:`device_profile.json` and :file:`wireless_device.json` files.

   .. code-block:: console

        $ cp <device_path>/device_profile.json <device_path>/wireless_device.json ./

   See an example path:

   .. code-block:: console

        $ cp /home/user/sidewalk_devices/device_profile.json /home/user/sidewalk_devices/wireless_device.json ./

#. Generate the manufacturing page BIN file.

   .. code-block:: console

       $ python3 provision.py aws --wireless_device_json wireless_device.json --device_profile_json device_profile.json --config config/nordic/nrf528xx_dk/config.yaml --output_bin mfg.bin

#. Convert the BIN file into a HEX file.

   .. code-block:: console

       # 0xFF000 is the current Sidewalk manufacturing storage partition start address.
       # sid_mfg_storage_partition is defined in sidewalk/samples/template/boards/nrf52840dk_nrf52840.overlay
       $ ${GNU_INSTALL_ROOT}/arm-none-eabi-objcopy -I binary -O ihex --change-addresses 0xFF000 mfg.bin mfg.hex

   Verify if the files exists:

   .. code-block:: console

       $ ls mfg*
       mfg.bin  mfg.hex

#. Flash the HEX file with nrfjprog by running the following command:

    .. code-block:: console

	$ nrfjprog --program mfg.hex --sectorerase --reset

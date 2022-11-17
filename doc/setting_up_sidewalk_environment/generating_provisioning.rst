.. _generate_manufacturing_data:

Generating device manufacturing data
************************************

.. note::
   Before configuring your HDK, ensure you have successfully completed the following steps:

    * :ref:`generating_app_server_keys` - Where you generated the public key
    * :ref:`endpoints_config_provisioning` - Where you obtained a certificate JSON file

In order to configure the manufacturing page, you need to use the certificate information from the device key bundle, and then flash it to the device.
In order to flash the keys onto your Sidewalk Endpoint, you have to use the :file:`provision.py` script provided in the Sidewalk software package with the device certificates in the working directory.

#. Go to :file:`provision` folder.

   .. code-block:: console

       $ cd ${ZEPHYR_BASE}/../sidewalk/tools/provision

#. Copy the application server public key from the :file:`application server cert` tool folder you generated in step :ref:`generating_app_server_keys`.

   .. code-block:: console

      $ cp ../application_server_cert/app-server-ed25519.public.bin ./

#. Move or copy the device key bundle to the provisioning tool's folder alongside your application server certificate.

   .. code-block:: console

          $ cp <certificate_path>/certificate_<SIDEWALK_ID>.json ./

   See an example path:

   .. code-block:: console

          $ cp /home/user/Downloads/certificate_BFFFFFFFFF.json ./

#. Open the :file:`README` file and follow the instructions to install software requirements.

   .. code-block:: console

       pip3 install --user -r requirements.txt

#. Generate the manufacturing page BIN file with the newly extracted device private keys and the application server public key generated in :ref:`generating_app_server_keys`.

   .. code-block:: console

       $ python3 provision.py acs --json certificate_<SIDEWALK_ID>.json --app_srv_pub app-server-ed25519.public.bin --config config/nordic/nrf528xx_dk/config.yaml --output_bin mfg.bin

#. Convert the BIN file into HEX and then flash it with nrfjprog, using the following commands:

   .. code-block:: console

       # 0xFF000 is the current Sidewalk manufacturing storage partition start address.
       # sid_mfg_storage_partition is defined in sidewalk/samples/template/boards/nrf52840dk_nrf52840.overlay
       $ ${GNU_INSTALL_ROOT}/arm-none-eabi-objcopy -I binary -O ihex --change-addresses 0xFF000 mfg.bin mfg.hex

       $ ls mfg*
       mfg.bin  mfg.hex


.. _Manufacturing: https://developer.amazon.com/acs-devices/console/sidewalk/docs/group__manufacturing.html
.. _My Products: https://developer.amazon.com/acs-devices/console/sidewalk/products

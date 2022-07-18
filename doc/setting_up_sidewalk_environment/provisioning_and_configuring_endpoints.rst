.. _endpoints_config_and_provisioning:

Provisioning and configuring Sidewalk Endpoints
###############################################

You need to provision your development board as a Sidewalk Endpoint.
You can do this by generating device and application server certificates with Sidewalk console and Sidewalk tools.
Once completed, you need to deploy them to the Sidewalk Endpoint.
These certificates provide the basis for mutual authentication and session establishment with the Sidewalk Network Server along with the 3P Application Service when a Sidewalk Endpoint connects to the Network Server.

.. note::
    For information on how to provision Sidewalk capable devices during mass production, refer to `Manufacturing`_.

A Sidewalk template sample is included with the device SDK and serves as a cloud reference application for developers.

.. _endpoints_config_provisioning:

Provisioning Sidewalk Endpoints
*******************************

#. Go to a list of products in `My Products`_ section in the Sidewalk console and select previously created product.

   .. figure:: /images/Step5-ProductList.jpg

#. Click :guilabel:`Configure Your Cloud`.

   .. figure:: /images/Step5-ConfigureCloud.jpg

#. Click :guilabel:`Provision Device`.

   .. figure:: /images/Step5-ProvisionButton.jpg

#. Choose :guilabel:`Generate New Certificate`.
   This will provision new Sidewalk Endpoints and generate their public and private certificates.

   .. figure:: /images/Step5-GenNewCertButton.jpg

#. Note down the Sidewalk ID of this Sidewalk Endpoint.

   .. note::
    Sidewalk ID needs to be used for flashing the manufacturing page (contains the certificates) to the hardware.
    It is also required in order to send data to your Sidewalk Endpoint.

   .. figure:: /images/Step5-SidewalkID.jpg

#. Download the device key bundle by clicking :guilabel:`Download Certificate`.
   The key bundle will be flashed onto the Sidewalk Endpoint.

   .. figure:: /images/Step5-DownloadCerts.jpg

.. _endpoints_config_hdk:

Configuring your Nordic Hardware Development Kit
************************************************

.. note::
   Before configuring your HDK ensure you have successfully completed steps outlined in :ref:`setting_up_dk`, as you need to have a working sdk-sidewalk.

In order to configure the manufacturing page, you need to use the certificate information from the device key bundle and then flash it to the device.
In order to flash the keys onto your Sidewalk Endpoint, you have to use the :file:`provision.py` script provided in the Sidewalk software package with the device certificates in the working directory.

.. _endpoints_config_flashing_certs:

Flashing the certificates
=========================

#. Go to provision tool folder.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/tools/provision

#. Move or copy the device key bundle to the provisioning tool's folder alongside your application server certificate.

   .. code-block:: console

          $ cp /path/to/certificate_${SIDEWALK_ID}.json ./

#. Open the :file:`README` file and follow the instructions to install software requirements.

   .. code-block:: console

       pip3 install --user -r requirements.txt

#. Generate the manufacturing page bin file with the newly extracted device private keys and the application server public key generated in :ref:`designing_sidewalk_product`.

   .. code-block:: console

       $ python3 provision.py acs --json certificate_${SIDEWALK_ID}.json \
       --app_srv_pub app-server-ed25519.public.bin \
       --config config/nordic/nrf528xx_dk/config.yaml --output_bin mfg.bin

#. Convert the bin file into hex and then flash it with nrfjprog, using the following commands:

   .. code-block:: console

       # 0xFF000 is the current Sidewalk manufacturing storage partition start address.
       # sid_mfg_storage_partition is defined in sidewalk/samples/template/boards/nrf52840dk_nrf52840.overlay
       $ ${GNU_INSTALL_ROOT}/arm-none-eabi-objcopy -I binary -O ihex \
       --change-addresses 0xFF000 mfg.bin mfg.hex

       $ ls mfg*
       mfg.bin  mfg.hex




.. _Manufacturing: https://developer.amazon.com/acs-devices/console/sidewalk/docs/group__manufacturing.html
.. _My Products: https://developer.amazon.com/acs-devices/console/sidewalk/products

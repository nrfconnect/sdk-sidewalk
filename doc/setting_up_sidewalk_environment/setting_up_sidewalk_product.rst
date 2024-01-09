.. _setting_up_sidewalk_product:

Setting up your Sidewalk product
################################

To create Sidewalk device follow the Amazon Sidewalk documentation: https://docs.aws.amazon.com/iot/latest/developerguide/sidewalk-gs-workflow.html

Provisioning generation
***********************

Tools for provisioning are included in the NCS repository in :file:`sidewalk/tools/provision`

Usage of the tool is described at https://docs.sidewalk.amazon/provisioning/iot-sidewalk-provision-endpoint.html

.. note::
   The default address of the mfg.hex in the Amazon instruction is incompatible with NCS applications.
   Add '--addr 0xFF000' argument to the provision.py script to generate mfg.hex compatible with NCS memory map.

If you're using the combined device JSON file that you obtained from the AWS IoT console, use the certificate_json parameter to specify this file as input when running the provisioning script.

python3 provision.py aws --output_bin mfg.bin --certificate_json certificate.json \ 
    --config config/[device_vendor]/[device]_dk/config.yaml --addr 0xFF000

If you're using the separate device JSON files that you obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the wireless_device_json and device_profile_json parameters to specify these files as input when running the provisioning script.

python3 provision.py aws --output_bin mfg.bin \  
    --wireless_device_json wireless_device.json \
    --device_profile_json device_profile.json \ 
    --config config/[device_vendor]/[device]_dk/config.yaml --addr 0xFF000


Flash the generated :file:`nordic_aws_nrf52840.hex` file with the provisioning data:

      .. code-block:: console

         $ nrfjprog --sectorerase --program nordic_aws_nrf52840.hex --reset

      .. note::
         If you reflashed the :file:`nordic_aws_nrf52840.hex` file on an already working device, make sure to perform a factory reset (**Button 1** long press) to deregister the previously flashed device.
         This will allow you to register a new product (new :file:`nordic_aws_nrf52840.hex`) in the Sidewalk network.

      .. note::
         The mfg.hex generated from the provision.py is compatible with all supported boards, even tho the name sugests only nrf52840.


Add MQTT to destination
***********************

#. Log in to `AWS`_.
   Open the AWS IoT Core service.

   .. figure:: /images/AWSIoTCore.png

#. Navigate to :guilabel:`Manage → LPWAN devices → Destinations`.

   .. figure:: /images/AWSIoTCoreDestinations.png

#. Select your destination end press :guilabel:`Edit`.

   .. figure:: /images/AWSIoTCoreDestinationEdit.png

#. Edit the destination details.

    a. Select :guilabel:`Publish to AWS IoT Core message broker`.
    #. Type your MQTT Topic name.

       .. figure:: /images/AWSIoTCoreDestinationTestMQTT.png

    #. Edit permissions.
    #. Select :guilabel:`Create a new service role` and click :guilabel:`Save`.

       .. figure:: /images/AWSIoTCoreDestinationTestRole.png

MQTT client
***********

#. Open the MQTT test client.
#. Type your MQTT topic in the filter field and click :guilabel:`Subscribe`.

   .. figure:: /images/AWSIoTCoreMQTT.png

.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app
.. _Amazon Sidewalk IoT Prerequisites: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#prerequisites
.. _Install virtual environment: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#1-install-virtual-environment
.. _AWS: https://aws.amazon.com/

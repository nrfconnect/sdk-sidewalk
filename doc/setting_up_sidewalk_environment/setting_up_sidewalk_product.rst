.. _setting_up_sidewalk_product:

Setting up your Sidewalk product
################################

.. contents::
   :local:
   :depth: 2

To correctly set up your Sidewalk device, first you have to onboard it.
Complete the `Onbarding your Sidewalk devices`_ steps described in the Amazon Sidewalk documentation.

Generate provisioning
*********************

The tools required for provisioning are located in the downloaded `sdk-nrf`_ repository under the :file:`sidewalk/tools/provision` path.

1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

   .. note::
      The default address of the the :file:`mfg.hex` file provided in the official Amazon Sidewalk documentation is incompatible with the nRF Connect SDK applications.
      You must add the ``--addr 0xFF000`` argument to the :file:`provision.py` script in order to generate the :file:`mfg.hex` file that is compatible with the nRF Connect SDK memory map.

   * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
     It will specify this file as an input when running the provisioning script.

      .. code-block:: python

         python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0xFF000

   * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
     This will specify both files as input when running the provisioning script.

      .. code-block:: python

         python3 provision.py nordic aws --output_bin mfg.bin \
            --wireless_device_json wireless_device.json \
            --device_profile_json device_profile.json \
             --addr 0xFF000

   .. note::
      The default name of the :file:`mfg.hex` file generated from the :file:`provision.py` script is :file:`nordic_aws_nrf52840.hex`.
      It is, however, compatible with other supported boards.

#. Flash the generated :file:`nordic_aws_nrf52840.hex` file with the provisioning data:

   .. code-block:: console

      $ nrfjprog --sectorerase --program nordic_aws_nrf52840.hex --reset

  * If you reflashed the :file:`nordic_aws_nrf52840.hex` file on an already working device, you need to deregister the previously flashed device.
    To do this, perform a factory reset by long pressing **Button 1**.
    This will allow you to register a new product (new :file:`nordic_aws_nrf52840.hex`) in the Sidewalk network.

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

Use MQTT client
***************

#. Open the MQTT test client.
#. Type your MQTT topic in the filter field and click :guilabel:`Subscribe`.

   .. figure:: /images/AWSIoTCoreMQTT.png

.. _Onbarding your Sidewalk devices: https://docs.aws.amazon.com/iot/latest/developerguide/sidewalk-gs-workflow.html
.. _Provision your Sidewalk endpoint and flash the binary image: https://docs.sidewalk.amazon/provisioning/iot-sidewalk-provision-endpoint.html
.. _Amazon Sidewalk Sample IoT App: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app
.. _Amazon Sidewalk IoT Prerequisites: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#prerequisites
.. _Install virtual environment: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#1-install-virtual-environment
.. _AWS: https://aws.amazon.com/
.. _sdk-nrf: https://github.com/nrfconnect/sdk-nrf

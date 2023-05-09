.. _setting_up_sidewalk_product:

Setting up your Sidewalk product
################################

To complete the set up of your Sidewalk product, you have to generate provisioning and add MQTT to destination.

Preconditions
*************

Before creating a Sidewalk provisioning file, you have to:

 * Create an AWS account
 * Set up an AWS user with permissions to create resources
 * Set up user's AWS credentials file on your local machine
 * Complete the `Amazon Sidewalk IoT Prerequisites`_ instructions


Provisioning generation
***********************

#. Go to AWS IoT Core for Sidewalk tools:

   .. code-block:: console

      cd tools/aws-iot-core-for-sidewalk

#. Populate the :file:`config.yaml` configuration file.
   Set `NORDIC` hardware platform:

   .. code-block:: yaml

      Config:
        AWS_PROFILE: default  # Name of your AWS profile from .aws/credentials
        DESTINATION_NAME: TemplateAppDestination  # Sidewalk destination used for uplink traffic routing
        HARDWARE_PLATFORM: NORDIC  # Available values: NORDIC, TI, SILABS or ALL
        USERNAME: null
        PASSWORD: null
        INTERACTIVE_MODE: True
      Outputs:
          DEVICE_PROFILE_ID: null
          WEB_APP_URL: null
      _Paths:
          PROVISION_SCRIPT_DIR: tools/provision
          SILABS_COMMANDER_TOOLS_DIR: null  # Not needed if Silabs Commander is already in system Path. Only needed for SILABS.

#. Run device provisioning scripts:

   a. Set up Python virtual environment for the provisioning tools:

      .. tabs::

         .. tab:: Linux

            .. code-block:: console

               python3 -m pip install --user virtualenv
               python3 -m virtualenv sample-app-env
               source sample-app-env/bin/activate
               python3 -m pip install --upgrade pip
               python3 -m pip install -r requirements.txt
               python3 -m pip install pyjwt -t ./ApplicationServerDeployment/lambda/authLibs

         .. tab:: Windows
      
            .. code-block:: console

               py -m pip install --user virtualenv
               py -m virtualenv sample-app-env
               sample-app-env\Scripts\activate.bat
               py -m pip install --upgrade pip
               py -m pip install -r requirements.txt
               py -m pip install pyjwt -t ./ApplicationServerDeployment/lambda/authLibs

   #. Run the device provisioning scripts:

      .. code-block:: console
   
         python3 EdgeDeviceProvisioning/provision_sidewalk_end_device.py

      You should see the following output:

      .. code-block:: console

         INFO:root:Status: 200
         INFO:root:Saving wireless device to file
         INFO:root:Generating MFG by calling provision.py
         INFO:root:  Generating MFG.hex for Nordic
         INFO:root:Done!

   #. Exit the Python virtual environment:

      .. code-block:: console

         deactivate

#. Flash the :file:`Nordic_MFG.hex` file.

   Your provisioning file is located in the :file:`EdgeDeviceProvisioning` directory.
   Devices are grouped in the device profile's subdirectory as shown in the structure below:

   .. code-block:: console

      EdgeDeviceProvisioning \
      - DeviceProfile_<profile-id> \
         - DeviceProfile.json
         - WirelessDevice_<device-id>\
             --  Nordic_MFG.bin
             --  Nordic_MFG.hex
             --  WirelessDevice.json

   a. Go to the device subdirectory:

      .. code-block:: console

         cd EdgeDeviceProvisioning/DeviceProfile_<profile-id>/WirelessDevice_<device-id>

      For example:

      .. code-block:: console

         cd EdgeDeviceProvisioning/DeviceProfile_102d750c-e4d0-4e10-8742-ea3698429ca9/WirelessDevice_5153dd3a-c78f-4e9e-9d8c-3d84fabb8911

   #. Flash the :file:`Nordic_MFG.hex` file with the provisioning data: 

      .. code-block:: console

         $ nrfjprog --sectorerase --program Nordic_MFG.hex --reset

      .. note::
         If you reflashed the :file:`Nordic_MFG.hex` file on an already working device, make sure to perform a factory reset (**Button 1** long press) to deregister the previously flashed device.
         This will allow you to register a new product (new :file:`Nordic_MFG.hex`) in the Sidewalk network.

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

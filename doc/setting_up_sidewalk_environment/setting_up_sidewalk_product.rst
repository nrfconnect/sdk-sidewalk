.. _setting_up_sidewalk_product:

Setting up your Sidewalk product
################################

To complete the set up of your Sidewalk product, you have to download the `Amazon Sidewalk Sample IoT App`_ repository, generate provisioning, and add MQTT to destination.

Downloading repository
**********************

#. Download `Amazon Sidewalk Sample IoT App`_ with EdgeDeviceProvisioning.
   Open the folder to which you want to clone the repository, and run the following command:

   .. code-block:: console

      $ git clone https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app.git
      $ cd aws-iot-core-for-amazon-sidewalk-sample-app

#. Complete the instructions in `Prerequisites`_ and `Install virtual environment`_ sections.

Generating provisioning
***********************

#. Populate the :file:`config.yaml` configuration file located in the ``aws-iot-core-for-amazon-sidewalk-sample-app`` directory.
   Fill out your credentials and the details of your device:

   .. code-block:: console

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

   .. code-block:: console

      $ python3 EdgeDeviceProvisioning/provision_sidewalk_end_device.py

   You should see the following output:

   .. code-block:: console

      EdgeDeviceProvisioning \
      - DeviceProfile_102d750c-e4d0-4e10-8742-ea3698429ca9 \
         - DeviceProfile.json
         - WirelessDevice_5153dd3a-c78f-4e9e-9d8c-3d84fabb8911\
             --  Nordic_MFG.bin
             --  Nordic_MFG.hex
             --  Nordic_NCS_MFG.bin
             --  Nordic_NCS_MFG.hex
             --  WirelessDevice.json

#. Flash the :file:`Nordic_NCS_MFG.hex` file:

   .. code-block:: console

       $ nrfjprog -f nrf52 --sectorerase --program EdgeDeviceProvisioning/DeviceProfile_102d750c-e4d0-4e10-8742-ea3698429ca9/WirelessDevice_5153dd3a-c78f-4e9e-9d8c-3d84fabb8911/Nordic_NCS_MFG --reset

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
.. _Prerequisites: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#prerequisites
.. _Install virtual environment: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app#1-install-virtual-environment
.. _AWS: https://aws.amazon.com/

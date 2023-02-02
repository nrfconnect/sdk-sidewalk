.. _designing_sidewalk_product_cli:

Designing your Sidewalk product
###############################

To design a Sidewalk device you need to create the AWS IoT resources, such as device, device profile or destination. 
AWS IoT Core for Sidewalk assigns a unique identifier to the resources that you create. 
You can, however, give them more descriptive names, add a description, or optional tags to easily identify and manage them. 
To design your Sidewalk devices, you can use the `AWS CLI`_.

Preconditions
*************

To design your Sidewalk product you need to use the Sidewalk console to define and create a new configuration for a given device.
Then, you need to link your developer and AWS cloud accounts.
For more instructions on onboarding your Sidewalk device, refer to the `Amazon Sidewalk Integration for AWS IoT Core`_ documentation.

.. note::
    Make sure your AWS CLI version supports Sidewalk commands.


Create Sidewalk Device in AWS IoT
*********************************

#. Create a Device Profile.

   .. code-block:: console

      $ aws iotwireless create-device-profile --name sidewalk_profile --sidewalk {}

   You can list the device profiles by running the following command:

   .. code-block:: console

      $ aws iotwireless list-device-profiles --device-profile-type Sidewalk

   The command will return your Device Profile Arn and Id:

   .. code-block:: json

      {
         "Arn": "arn:aws:iotwireless:us-east-1:112233445566:DeviceProfile/12345678-000-000-000-87654321",
         "Name": "sidewalk_profile",
         "Id": "12345678-000-000-000-87654321"
      }

#. Get the Device Profile.

   Use the generated ``"Id"`` as your ``<DEVICE_PROFILE_ID>``.

   .. code-block:: console

      $ aws iotwireless get-device-profile --id "<DEVICE_PROFILE_ID>" > device_profile.json

   See an example:

   .. code-block:: console

      $ aws iotwireless get-device-profile --id "12345678-000-000-000-87654321" > device_profile.json

#. Create a Sidewalk Wireless Device.

   a. Use the Device Profile ID you generated and saved it the :file:`device_profile.json` file.

   #. Use ``DestinationName`` from the destination list, or provide any string value (AWS will create an empty destination that you can update later).

      .. code-block:: console

         $ aws iotwireless list-wireless-devices --wireless-device-type Sidewalk

   #. Create Wireless Device by running the following command:
   
      .. code-block:: console

         $ aws iotwireless create-wireless-device --type "Sidewalk" --name sidewalk_device  --destination-name "SidewalkDestination"  --sidewalk DeviceProfileId="<DEVICE_PROFILE_ID>"

   The command will return wireless device Arn and Id of your wireless device:
 
   .. code-block:: json

      {
         "Arn": "arn:aws:iotwireless:us-east-1:112233445566:WirelessDevice/01234567-111-222-111-76543210",
         "Id": "01234567-111-222-111-76543210"
      }

   You can see the list of wireless devices by running the following command:

   .. code-block:: console

      $ aws iotwireless list-wireless-devices --wireless-device-type Sidewalk

#. Get a Sidewalk Wireless Device

   .. code-block:: console

      $ aws iotwireless get-wireless-device --identifier-type WirelessDeviceId  --identifier "<WIRELESS_DEVICE_ID>" > wireless_device.json

   See an example:

   .. code-block:: console

      $ aws iotwireless get-wireless-device --identifier-type WirelessDeviceId  --identifier "01234567-111-222-111-76543210" > wireless_device.json

   The :file:`wireless_device.json` file should look as follows:

   .. code-block:: json

      {
          "Type": "Sidewalk",
          "Name": "sidewalk_device",
          "DestinationName": "SidewalkDestination",
          "Id": "01234567-111-222-111-76543210",
          "Arn": "arn:aws:iotwireless:us-east-1:112233445566:WirelessDevice/01234567-111-222-111-76543210",
          "Sidewalk": {
              "SidewalkManufacturingSn": "012345678901234567890123456789AABBCCDDEE",
              "DeviceCertificates": [
                  {
                      "SigningAlg": "Ed25519",
                      "Value": "<certificate value>"
                  },
                  {
                      "SigningAlg": "P256r1",
                      "Value": "<certificate value>"
                  }
              ],
              "PrivateKeys": [
                  {
                      "SigningAlg": "Ed25519",
                      "Value": "<signature value>"
                  },
                  {
                      "SigningAlg": "P256r1",
                      "Value": "<signature value>"
                  }
              ],
              "DeviceProfileId": "12345678-000-000-000-87654321",
              "CertificateId": "0123456789ABCDEF0123456789AB",
              "Status": "PROVISIONED"
          }
      }

.. _AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-started.html
.. _Amazon Sidewalk Integration for AWS IoT Core: https://docs.aws.amazon.com/iot/latest/developerguide/iot-sidewalk.html

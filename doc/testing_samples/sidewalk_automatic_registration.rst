.. _automatic_registration_sidewalk_endpoints:

Automatic Touchless Registration using Sidewalk FFN
###################################################

Touchless registration using Sidewalk FFN is a silent, automatic process that occurs between an unregistered Sidewalk Endpoint and a Sidewalk Gateway over Bluetooth LE.
The endpoint sends Bluetooth LE beacons that are received by the Sidewalk Gateway when the endpoint and gateway are in close proximity.

Ensure you Sidewalk Gateway is opted into Sidewalk, powered on and in close range to your Sidewalk Endpoint.
Successful registration is indicated by a log message ``Device Is registered`` as shown below.

   .. code-block:: console

      *** Booting Zephyr OS build v3.0.99-ncs1-4913-gf7b06162027d  ***
      ----------------------------------------------------------------
      sidewalk             v1.13.0-6-g2e0691d-dirty
      nrf                  v2.0.0-734-g3904875f6
      zephyr               v3.0.99-ncs1-4913-gf7b0616202-dirty
      ----------------------------------------------------------------
      sidewalk_fork_point = 92dcbff2da68dc6853412de792c06cc6966b8b79
      build time          = 2022-11-17 10:52:50.833532+00:00
      ----------------------------------------------------------------

      [00:00:00.006,225] <inf> sid_template: Sidewalk example started!
      [00:00:00.061,584] <inf> sid_thread: Initializing sidewalk - Build with Link mask LoRa
      [00:00:00.061,614] <inf> sid_thread: Start Sidewalk link_mask:1
      [00:00:00.438,751] <inf> sdc_hci_driver: SoftDevice Controller build revision:
                                               b6 5a b4 2c 4d fa 65 96  62 bc a6 bf 2e f3 36 5c |.Z.,M.e. b.....6\
                                               c6 ab 31 a6                                      |..1.
      [00:00:00.477,325] <inf> sid_thread: status changed: not ready
      [00:00:00.477,355] <inf> sid_thread: Device Unregistered, Time Sync Fail, Link status Down
      [00:00:00.477,355] <inf> sid_thread: Starting Sidewalk thread ...
      [00:00:11.135,070] <inf> sid_thread: status changed: not ready
      [00:00:11.135,131] <inf> sid_thread: Device Unregistered, Time Sync Fail, Link status Down
      [00:00:31.045,440] <inf> sid_thread: status changed: not ready
      [00:00:31.045,471] <inf> sid_thread: Device Is registered, Time Sync Fail, Link status Up
      [00:00:31.526,733] <inf> sid_thread: status changed: not ready


De-registering your Sidewalk Endpoint
=====================================

Under certain situations, you may wish to de-register your endpoint.
For example, if you want to test the registration process multiple times, you will need to de-register your endpoint before attempting to re-register it.

#. Find ``<AWS_ACCESS_KEY>`` and ``<AWS_SECRET_KEY>`` with your AWS credentials.
   These are the credentials you noted down during setting up your AWS account.

   a. In case you need to obtain new keys, navigate to :guilabel:`IAM`, and select :guilabel:`Users`.
   #. Under your user account go to :guilabel:`Security Credentials` tab, and click :guilabel:`Create access key button`.

   .. figure:: /images/AWSDeregisterSecurityCredentials.png

#. Replace ``<YOUR WIRELESS DEVICE ID>`` variable with your own wireless device ID.
   You can find it listed as :guilabel:`Device ID` under :guilabel:`AWS IoT` → :guilabel:`Manage` → :guilabel:`LPWAN devices` → :guilabel:`Devices`.

   .. figure:: /images/AWSDeregisterWirelessId.png

#. Run the following command on your console:

   .. code-block:: console

     curl -X PATCH "https://api.iotwireless.us-east-1.amazonaws.com/wireless-devices/<YOUR WIRELESS DEVICE ID>/deregister?WirelessDeviceType=Sidewalk" --user <AWS_ACCESS_KEY>:<AWS_SECRET_KEY> --aws-sigv4 "aws:amz:us-east-1:iotwireless"

   API response code should be 200 or 202.

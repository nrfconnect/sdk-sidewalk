.. _sidewalk_testing:

Testing Sidewalk
################

With the sample application, you will be able to establish a connection with Sidewalk Gateways and to transfer and receive data over Sidewalk.
Additionally, you will be able to send a message to your Sidewalk Endpoint using AWS CLI and to verify if it was successfully delivered.

.. _sidewalk_testing_starting:

Starting Sidewalk
*****************

To start Sidewalk, do the following:

#. Connect your Nordic device to the PC via USB.
   Set the power switch on the device to **ON**.

#. Flash the sample application with manufacturing data as described in the :ref:`samples_building_and_running` documentation.

   You should see the following logs:

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

#. Wait for the device to register, or perform registration manually as described in the :ref:`registering_sidewalk` documentation.

   You should see the following logs:

   .. code-block:: console

      [00:00:31.045,471] <inf> sid_thread: Device Is registered, Time Sync Fail, Link status Up

#. Wait for the device to complete time sync with the Sidewalk network.

   You should see the following logs:

   .. code-block:: console

      [00:00:35.827,789] <inf> sid_thread: status changed: ready
      [00:00:35.827,850] <inf> sid_thread: Device Is registered, Time Sync Success, Link status Up

#. Perform a button action.

   a. For a Bluetooth LE device, you need to send a connection request before sending a message.
      Press **Button 2** to set the request.

      .. code-block:: console

         [00:44:42.347,747] <inf> sid_thread: Set connection request

      Wait for status change.

      When Sidewalk status changes, **LED 1** turns on.
      The following messages appear in the log:

      .. code-block:: console

          [00:45:31.597,564] <inf> sid_thread: status changed: init
          [00:45:31.597,564] <dbg> sid_thread: on_sidewalk_status_changed: Device Is registered, Time Sync Success, Link status Up

   #. For a LoRa device, press any button except the factory reset button.
      To switch the transport protocol from Bluetooth LE to LoRa, press **Button 2**.

      .. code-block:: console

         [00:00:09.089,385] <inf> sid_template: Pressed button 2
         [00:00:09.089,508] <inf> sid_thread: Start Sidewalk link_mask:4
         [00:00:09.089,874] <inf> sid_thread: status changed: not ready
         [00:00:09.089,904] <inf> sid_thread: Device Is registered, Time Sync Fail, Link status Down
         [00:00:09.537,963] <inf> sid_thread: Device Is registered, Time Sync Fail, Link status Up
         [00:00:09.537,963] <inf> sid_thread: Link mode cloud, on lora
         [00:00:09.537,994] <err> sid_thread: Option failed (err -11)

    Wait for status change.

    When Sidewalk status changes, **LED 1** turns on.
    The following messages appear in the log:

      .. code-block:: console

         [00:00:21.362,884] <inf> sid_thread: status changed: ready
         [00:00:21.362,915] <inf> sid_thread: Device Is registered, Time Sync Success, Link status Up
         [00:00:21.362,945] <inf> sid_thread: Link mode cloud, on lora

.. _sidewalk_testing_send_message:

Sending message to AWS MQTT
***************************

You can use `AWS IoT MQTT client`_ to view the received and republished messages from the device.
Follow the outlined steps:

#. Enter ``#`` and click :guilabel:`Subscribe to topic`.
   You are now subscribed to the republished device messages.

#. To see the data republished into the subscribed MQTT topic, press **Button 3** on your development kit.

   .. code-block:: console

      # Logs from DK after pressing "Button 3"
      [00:04:57.461,029] <inf> sid_template: Pressed button 3
      [00:04:57.461,120] <inf> sid_thread: sending counter update: 0
      [00:04:57.461,456] <inf> sid_thread: queued data message id:3


      # Logs from MQTT test client
      "WirelessDeviceId": "a6e53628-ffc8-4320-9461-cf5c7997bf17",
      "PayloadData": "MDA=",
      "WirelessMetadata": {
         "Sidewalk": {
            "CmdExStatus": "COMMAND_EXEC_STATUS_UNSPECIFIED",
            "MessageType": "CUSTOM_COMMAND_ID_NOTIFY",
            "NackExStatus": [],
            "Seq": 2,
            "SidewalkId": "BFFFFFFFFF"
         }
      }

   Payload data is presented in base64 format.
   You can check it using the Linux base64 tool:

   .. code-block:: console

      $ echo -n "MDA=" | base64 --decode
      00

   Data is republished into the subscribed MQTT topic.

   .. figure:: /images/Step7-MQTT-Subscribe.png

.. _sidewalk_testing_receive_message:

Receiving message from AWS MQTT
*******************************

#. To be able to use AWS CLI, ensure you completed steps in the `Installing or updating the latest version of the AWS CLI`_ documentation.

#. Ensure your AWS user account or IAM role has the corresponding AWSIoTWireless permission policy.

   .. figure:: /images/sidewalk_iam_iotwireless_policy.png

   For more information on how to change permissions for an IAM user, see the `ID users change permissions`_ documentation.

#. Run the following command to send a message to your Sidewalk Endpoint, where:

   * :guilabel:`Wireless-Device-ID` is the ID of the wireless devices listed in AWS IoT Core,
   * the Seq integer should be different for each subsequent request,
   * the payload data is base64 encoded.

#. Prepare message payload in the base64 format.

   .. code-block:: console

      $ echo -n "Hello   Sidewalk!" | base64
      SGVsbG8gICBTaWRld2FsayE=

#. Send message using AWS tools.

   .. code-block:: console

      $ aws iotwireless send-data-to-wireless-device --id=f1b4a9f5-4bf0-41ae-a383-5007c8340969 --transmit-mode 0 --payload-data="SGVsbG8gICBTaWRld2FsayE=" --wireless-metadata "Sidewalk={Seq=1}"

   .. note::
      Ensure to increase 'Seq' number on every message.
      The device will not receive a message with lower or equal sequence number.

   Successfully sent response should look as follows:

   .. code-block:: console

      {
          "MessageId": "eabea2c7-a818-4680-8421-7a5fa322460e"
      }

   In case you run into the following error, ensure your IAM user or role has permissions to send data to your wireless device:

   .. code-block:: console

      {
         "Message": "User: arn:aws:iam::[AWS Account ID]:user/console_user is not authorized to perform:
         iotwireless:SendDataToWirelessDevice on resource: arn:aws:iotwireless:us-east-1:[AWS Account ID]:
         WirelessDevice/[Wireless Device ID]"
      }

   Data will be received in Sidewalk logs:

   .. code-block:: console

       [00:06:56.338,134] <inf> sid_thread: Message data:
                                     48 65 6c 6c 6f 20 20 20  53 69 64 65 77 61 6c 6b |Hello    Sidewalk
                                     21                                               |!

Testing Device Firmware Update (DFU)
************************************

#. To enter the DFU mode, long press **Button 4** on your development kit.
   This action disables the Sidewalk stack and starts the `Zephyr SMP Server`_.

#. To perform a firmware update, follow the Bluetooth testing steps from the `DevZone DFU guide`_.

#. To exit the DFU mode, reset your device.
   The device will restart in the Sidewalk mode.
   If the update completes successfully, the device will start a new image.
   However, if the update fails, you will still be able to see the old image.

Testing Power Profiles
**********************

Power profiles are available for sub-GHz radio communication, such as LoRa or FSK.
For more information about Sidewalk Power Profiles, refer to the Sidewalk Protocol Specification.

.. Link to the Sidewalk Protocol Specification page will be included during the release in March.

The following profiles are available in the template application:

+-------+-------------------+----------------------+--------------+-------------+
| Name  | Power consumption | Messages may be lost | LoRa profile | FSK profile |
+=======+===================+======================+==============+=============+
| Light | Lower             | Yes                  | A            | 1           |
+-------+-------------------+----------------------+--------------+-------------+
| Fast  | Higher            | No                   | B            | 2           |
+-------+-------------------+----------------------+--------------+-------------+

To test power profiles. complete the following steps:

#. Build and flash the template application with the LoRa or FSK link mask.

   .. code-block:: console

       [00:00:00.000,915] <inf> sid_template: Sidewalk example started!
       ----------------------------------------------------------------
       sidewalk             v1.14.0-6-gf586541-dirty
       nrf                  v2.2.0
       zephyr               v3.2.99-ncs1
       ----------------------------------------------------------------
       sidewalk_fork_point = 28f9d7955a00020715308d0c639013fc5f9db930
       build time          = 2023-02-01 10:44:17.326681+00:00
       ----------------------------------------------------------------
       [00:00:00.001,251] <inf> sid_template: Marked image as OK
       [00:00:00.063,476] <inf> sid_thread: Initializing sidewalk, built-in LoRa link mask

#. Switch to LoRa or FSK by short pressing **Button 3**.

   .. code-block:: console

       [00:00:07.487,487] <inf> button: button pressed 3 short
       [00:00:07.487,609] <inf> sid_thread: Start Sidewalk link_mask: LoRa

   Wait a few seconds until you see the following output:

   .. code-block:: console

       [00:00:15.017,425] <inf> sid_thread: Device Is registered, Time Sync Success, Link status Up
       [00:00:15.017,486] <inf> sid_thread: Link mode cloud, on lora

#. Set the power profile.
   Long press **Button 2** to switch between the light and fast profiles.

   .. code-block:: console

       [00:00:29.375,732] <inf> button: button pressed 2 short
       [00:00:29.375,854] <inf> sid_thread: Profile set fast
       [00:00:29.375,976] <inf> sid_thread: Profile set success.

#. Get the current profile by short pressing **Button 2**.

   .. code-block:: console

       [00:00:35.433,441] <inf> button: button pressed 2 long
       [00:00:35.433,654] <inf> sid_thread: Profile id 0x81
       [00:00:35.433,654] <inf> sid_thread: Profile dl count 0
       [00:00:35.433,685] <inf> sid_thread: Profile dl interval 5000
       [00:00:35.433,685] <inf> sid_thread: Profile wakeup 0


.. _AWS IoT MQTT client: https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html
.. _Installing or updating the latest version of the AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html
.. _ID users change permissions: https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_change-permissions.html
.. _DevZone DFU guide: https://devzone.nordicsemi.com/guides/nrf-connect-sdk-guides/b/software/posts/ncs-dfu#ble_testing
.. _Zephyr SMP Server: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/device_mgmt/ota.html#smp-server

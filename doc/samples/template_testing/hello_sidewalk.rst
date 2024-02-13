.. _sidewalk_hello:

Hello Sidewalk
##############

This sample demonstrates a simple Sidewalk End Node application.
It can send Hello message to Sidewalk cloud on button press, as well as represent the Sidewalk status by LED state.
It supports Bluetooth LE, LoRa, and FSK link modes.
External QSPI Flash is used for firmware updates.

Overview
********

The Sidewalk status is indicated by LEDs state and is also printed in the device logs.
It supports actions, such as sending messages, performing factory reset, and entering the DFU state.


.. _sidewalk_hello_user_interface:

User Interface
**************

Button 1 (short press):
   Send Hello -  This action queues a message to the cloud.
   If Sidewalk is not ready, it displays an error without sending the message.
   On Bluetooth LE, the application performs Connection Requests before sending.

Button 2 (short press):
   Set Connection Request - The device requests the Sidewalk Gateway to initiate a connection while the device advertises through Bluetooth LE.
   Gateways may not process this request, as it depends on the number of devices connected to it.

Button 3 (short press):
   Toggles the Sidewalk link mask - This action switches from Bluetooth LE to FSK, from FSK to LoRa, and from LoRa to Bluetooth LE.
   A log message informs about the link mask switch and the status of the operation.

LED 1:
   Application established a link and connected successfully.
   It is ready to send and receive Sidewalk messages.

LED 2:
   Application time synced successfully.

LED 3:
   Application registered successfully.

LED 4:
   Application woke up successfully.

For common sidewalk template user interface description see ::ref:`sidewalk_template_user_interface`

Configuration
*************

The hello Sidewalk application supports the following configurations:

* ``CONFIG_TEMPLATE_APP_ECHO_MSGS`` -- The sample echoes received Sidewalk messages of type GET and SET to the Sidewalk cloud.

.. _sidewalk_template_hello_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.

Wait for the device to complete the automatic registration.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

Starting Sidewalk
-----------------

To start Sidewalk, do the following:

#. Connect your Nordic device to the PC through USB.
   Set the power switch on the device to **ON**.

#. Flash the sample application with the manufacturing data as described in the Building and running section of the respective sample.

   You should see the following logs:

   .. code-block:: console

      *** Booting nRF Connect SDK 883c3709f9c8 ***
      ----------------------------------------------------------------
      sidewalk             v2.5.0-3-g1232aabb
      nrf                  v2.5.0-g271e80422
      zephyr               883c3709f9
      ----------------------------------------------------------------
      sidewalk_fork_point = af5d608303eb03465f35e369ef22ad6c02564ac6
      build time          = 2023-03-14 15:00:00.000000+00:00
      board               = nrf52840dk_nrf52840
      ----------------------------------------------------------------
      [00:00:00.001,373] <inf> application_state: working = true
      [00:00:00.055,480] <inf> sidewalk_app: Sidewalk link switch to BLE

   When Sidewalk sample starts, **LED 4** turns on.

#. Wait for the device to register, or perform registration manually as described in the :ref:`registering_sidewalk` documentation.

   You should see the following logs:

   .. code-block:: console

      [00:00:31.045,471] <inf> sid_thread: Device Is registered, Time Sync Fail, Link status Up

   When Sidewalk registration status changes, **LED 3** turns on.

#. Wait for the device to complete time sync with the Sidewalk network.

   You should see the following logs:

   .. code-block:: console

      [00:00:35.827,789] <inf> sid_thread: status changed: ready
      [00:00:35.827,850] <inf> sid_thread: Device Is registered, Time Sync Success, Link status Up

   When Sidewalk gets Time Sync, **LED 2** turns on.

#. Wait for status change.

   a. For a Bluetooth LE device, status change occurs on request. Gateway connects over Bluetooth LE before sending down-link message, and device sends connection request before up-link message. Sidewalk automatically disconnects Bluetooth LE after some inactivity period.

   #. For a LoRa and FSK device, the following messages appear in the log:

      .. code-block:: console

         [00:45:31.597,564] <inf> sid_thread: status changed: init
         [00:45:31.597,564] <dbg> sid_thread: on_sidewalk_status_changed: Device Is registered, Time Sync Success, Link status Up

   When Sidewalk Link Status is Up, **LED 1** turns on.

Sending message to AWS MQTT
---------------------------

You can use `AWS IoT MQTT client`_ to view the received and republished messages from the device.
Follow the outlined steps:

#. Enter ``#`` and click :guilabel:`Subscribe to topic`.
   You are now subscribed to the republished device messages.

#. To see the data republished into the subscribed MQTT topic, press **Button 1** on your development kit.

   .. code-block:: console

      # Logs from DK after pressing "Button 1"
      [00:04:57.461,029] <inf> sid_template: Pressed button 1
      [00:04:57.461,120] <inf> sid_thread: sending counter update: 0
      [00:04:57.461,456] <inf> sid_thread: queued data message id:3


      # Logs from MQTT test client
	  {
      "MessageId": "4c5dadb3-2762-40fa-9763-8a432c023eb5",
      "WirelessDeviceId": "5153dd3a-c78f-4e9e-9d8c-3d84fabb8911",
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
   You can decode it using Python:

   .. code-block:: console

      python -c "import sys,base64;print(base64.b64decode(sys.argv[1].encode('utf-8')).decode('utf-8'))" MDA=
      00

   Data is republished into the subscribed MQTT topic.

   .. figure:: /images/Step7-MQTT-Subscribe.png

Receiving message from AWS MQTT
-------------------------------

#. To be able to use AWS CLI, ensure you completed steps in the `Installing or updating the latest version of the AWS CLI`_ documentation.

#. Run the following command to send a message to your Sidewalk Endpoint:

   .. code-block:: console

      aws iotwireless send-data-to-wireless-device --id=<wireless-device-id> --transmit-mode 0 --payload-data="<payload-data>" --wireless-metadata "Sidewalk={Seq=<sequence-number>}"


   * ``<wireless-device-id>`` is the Wireless Device ID of your Sidewalk Device.

      You can find it in the :file:`WirelessDevice.json` file, generated with the :file:`Nordic_MFG.hex` file during :ref:`setting_up_sidewalk_product`.

      You can also find your Wireless Device ID in the message sent form the device to AWS, it you have sent it before.

   * ``<payload-data>`` is base64 encoded.

      To prepare a message payload in the base64 format, run:

      .. code-block:: console

         python -c "import sys,base64;print(base64.b64encode(sys.argv[1].encode('utf-8')).decode('utf-8'))" "Hello   Sidewalk!"
         SGVsbG8gICBTaWRld2FsayE=

   * ``<sequence-number>`` is an integer and should be different for each subsequent request.

      .. note::
         Ensure to increase 'Seq' number on every message.
         The device will not receive a message with lower or equal sequence number.

   Once you have populated the command with data, it should look similar to the following:

   .. code-block:: console

      aws iotwireless send-data-to-wireless-device --id=5153dd3a-c78f-4e9e-9d8c-3d84fabb8911 --transmit-mode 0 --payload-data="SGVsbG8gICBTaWRld2FsayE=" --wireless-metadata "Sidewalk={Seq=1}"

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


.. include:: testing_common_include.txt

.. include:: ../../ncs_links.rst

.. _AWS IoT MQTT client: https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html
.. _Installing or updating the latest version of the AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html
.. _ID users change permissions: https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_change-permissions.html
.. _DevZone DFU guide: https://devzone.nordicsemi.com/guides/nrf-connect-sdk-guides/b/software/posts/ncs-dfu#ble_testing
.. _Sidewalk Protocol Specification: https://docs.sidewalk.amazon/specifications/
.. _Sidewalk_Handler CloudWatch log group: https://console.aws.amazon.com/cloudwatch/home?region=us-east-1#logsV2:log-groups/log-group/$252Faws$252Flambda$252FSidewalk_Handler
.. _AWS IoT MQTT client: https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html
.. _CloudShell: https://console.aws.amazon.com/cloudshell
.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

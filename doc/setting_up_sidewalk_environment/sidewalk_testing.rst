.. _sidewalk_testing:

Testing with Sidewalk
#####################

This section provides guidelines on how to compile and run the Sidewalk template sample.
With the sample application, you will be able to establish a connection with Sidewalk Gateways and to transfer and receive data over Sidewalk.
Additionally, you will be able to send a message to your Sidewalk Endpoint using AWS CLI and verify if it was successfully delivered.

#. Connect the DK to your computer via USB.
#. Check the development kit serial port.

   .. code-block:: console

       ~$ nrfjprog --com
        683553359    /dev/ttyACM0    VCOM0

#. Connect to serial port from above with the following settings:

    * Baud rate: 115200
    * 8 data bits
    * 1 stop bit
    * No parity
    * HW flow control: None

   .. code-block:: console

       ~$ screen /dev/ttyACM0 115200
       *** Booting Zephyr OS build v3.0.99-ncs1  ***
       [00:00:00.004,241] <inf> sid_template: Sidewalk example started!
       [00:00:00.017,364] <inf> fs_nvs: 2 Sectors of 4096 bytes
       [00:00:00.017,395] <inf> fs_nvs: alloc wra: 0, f88
       [00:00:00.017,425] <inf> fs_nvs: data wra: 0, 60
       [00:00:00.199,340] <inf> sidewalk: DR state [3]
       [00:00:00.199,371] <dbg> sid_ble: ble_adapter_init: Enable BT
       [00:00:00.199,493] <inf> sdc_hci_driver: SoftDevice Controller build revision
       [00:00:00.239,044] <inf> sid_thread: status changed: is ready
       [00:00:00.239,074] <dbg> sid_thread: on_sidewalk_status_changed: Device Is registered, Time Sync Fail, Link status Down
       [00:00:00.239,074] <inf> sid_thread: Starting sidewalk thread ...

   As the binaries have been flashed in :ref:`endpoints_config_and_provisioning`, the Sidewalk sample will run automatically when turned on.
   It will send the device information to the Sidewalk Gateway upon pressing :guilabel:`Button 3` on the DK.

   See `NCS testing applications`_ for more information about NCS samples logging.

Confirming Data Reception
*************************

To check if the application sends data over Sidewalk, use the created earlier :guilabel:`SideWalkRuleDemo` which triggers :guilabel:`Sidewalk_Handler_Demo` lambda function and republishes the message to an AWS IoT topic.

#. Open `Sidewalk_Handler CloudWatch log group`_ and review the log sets corresponding to the timestamps for when you pressed the buttons on your DK.
#. View the received and republished messages from the device.
   You can use `AWS IoT MQTT client`_.

   a. Enter ``#`` and click :guilabel:`Subscribe to topic`.
      You are now subscribed to the republished device messages.
   #. For Bluetooth LE device, send connection request before sending a message.
      Press :guilabel:`Button 2` to set connection request.

      .. code-block:: console

         [00:44:42.347,747] <inf> sid_thread: Set connection request

   #. Wait on connection.

      When Sidewalk status changes, :guilabel:`LED 1` turns on.
      The following messages appears in the log:

      .. code-block:: console

          [00:45:31.597,564] <inf> sid_thread: status changed: init
          [00:45:31.597,564] <dbg> sid_thread: on_sidewalk_status_changed: Device Is registered, Time Sync Success, Link status Up


   #. To see the data republished into the subscribed MQTT topic, press :guilabel:`Button 3` on your DK.

      .. code-block:: console

         # Logs from DK after pressing "Button 3"
         [00:37:42.456,573] <inf> sid_thread: sending counter update: 1
         [00:37:42.456,787] <inf> sidewalk: find_route [RT:0] [CMD:8:0:2]
         [00:37:42.457,244] <inf> sidewalk: ENC: M:1d [1:0:2]
         [00:37:42.458,465] <inf> sidewalk: Tx Start (35)
         [00:37:42.458,587] <inf> sidewalk: Tx S:7 T:1 (0) I:0
         [00:37:42.458,923] <inf> sidewalk: data_send send 0
         [00:37:42.458,984] <dbg> sid_thread: send_message: queued data message id:2
         [00:37:42.605,377] <dbg> sid_ble_srv: notification_sent: Notification sent.
         [00:37:42.605,407] <dbg> sid_thread: on_sidewalk_event: on event, from ISR, context 0x20003968
         [00:37:42.605,499] <dbg> sid_thread: on_sidewalk_msg_sent: sent message(type: 2, id: 2)


         # Logs from MQTT test client
         {
         "WirelessDeviceId": "a6e53628-ffc8-4320-9461-cf5c7997bf17",
         "PayloadData": "MDE=",
         "WirelessMetadata": {
            "Sidewalk": {
               "CmdExStatus": "COMMAND_EXEC_STATUS_UNSPECIFIED",
               "MessageType": "CUSTOM_COMMAND_ID_NOTIFY",
               "NackExStatus": [],
               "Seq": 2,
               "SidewalkId": "BFFFFFFE09"
            }
         }
         }

   Data is republished into the subscribed MQTT topic.

   .. figure:: /images/Step7-MQTT-Subscribe.png

Sending data to Sidewalk Endpoint using AWS CLI
***********************************************

#. To be able to use AWS CLI, ensure you completed steps in `Installing or updating the latest version of the AWS CLI`_.

#. Ensure your AWS user account or IAM role has the corresponding AWSIoTWireless permission policy.

   .. figure:: /images/sidewalk_iam_iotwireless_policy.png

   For more information on how to change permissions for am IAM user, see `ID users change permissions`_.

#. Run the following command to send a message to your Sidewalk Endpoint, where:

   * :guilabel:`Wireless-Device-ID` is the ID of the wireless devices listed in AWS IoT Core
   * :guilabel:`The Seq integer should be different for each subsequent request` and the payload data is base64 encoded.

   .. code-block:: console

      aws iotwireless send-data-to-wireless-device --id=[Wireless-Device-ID] --transmit-mode 0
      --payload-data="SGVsbG8gICBTaWRld2FsayE=" --wireless-metadata "Sidewalk={Seq=1}"

   This command sends a message to your Sidewalk Endpoint.
   The actual ``payload-data`` needs to refer to your device manual.
   It ensures you are sending messages that can be understood by your devices.

   .. code-block:: console

      # Example command:
      $ aws iotwireless send-data-to-wireless-device --id=a6e53628-ffc8-4320-9461-cf5c7997bf17 --transmit-mode 0 --payload-data="SGVsbG8gICBTaWRld2FsayE=" --wireless-metadata "Sidewalk={Seq=3}"
      {
          "MessageId": "81dd2aa9-1b5d-4e4a-8df0-3527a5255a7b"
      }

      # Logs from DK
      [00:39:09.293,609] <dbg> sid_ble_ama_srv: ama_srv_on_write: Data received for AMA_SERVICE [len=84].
      [00:39:09.293,670] <dbg> sid_thread: on_sidewalk_event: on event, from ISR, context 0x20003968
      [00:39:09.313,812] <inf> sidewalk: New Rx S:7 T:10
      [00:39:09.313,995] <inf> sidewalk: Rx S:7 T:10 (78/78)
      [00:39:09.315,887] <inf> sidewalk: DEC: M:1d [8:0:2]
      [00:39:09.316,040] <inf> sidewalk: on_recv_ready [CMD:8:0:2]
      [00:39:09.316,131] <inf> sidewalk: on_recv_ready [RT:LOCAL]
      [00:39:09.316,162] <dbg> sid_thread: on_sidewalk_msg_received: received message(type: 2, id: 3 size 17)
      [00:39:09.316,192] <inf> sid_thread: Message data:
                                           48 65 6c 6c 6f 20 20 20  53 69 64 65 77 61 6c 6b |Hello    Sidewalk
                                           21                                               |!
      [00:39:09.316,375] <inf> sidewalk: Delete rx_buffer :: Gateway [ab 66] :: Stream [7] :: Transaction [10]

   If you use the same sequence ID, you will get the following output:

   .. code-block:: console

      [00:40:38.146,423] <dbg> sid_ble_ama_srv: ama_srv_on_write: Data received for AMA_SERVICE [len=68].
      [00:40:38.146,453] <dbg> sid_thread: on_sidewalk_event: on event, from ISR, context 0x20003968
      [00:40:38.221,527] <inf> sidewalk: New Rx S:7 T:11
      [00:40:38.221,710] <inf> sidewalk: Rx S:7 T:11 (62/62)
      [00:40:38.223,052] <inf> sidewalk: DEC: M:1d [8:0:2]
      [00:40:38.223,175] <inf> sidewalk: on_recv_ready [CMD:8:0:2]
      [00:40:38.223,266] <inf> sidewalk: on_recv_ready [RT:LOCAL]
      [00:40:38.223,449] <inf> sidewalk: Message duplicated: 4101000000 seqn=3
      [00:40:38.223,541] <inf> sidewalk: Duplicate detected, Disconnecting  BLE link

   If you run the ``send-data-to-wireless-device`` command in `CloudShell`_, you will get the following response:

   .. code-block:: console

      [cloudshell-user@ipxxxxx]$ aws iotwireless send-data-to-wireless-device \
      --id=646e6051-bd4a-45a9-a5d3-b8a242af2115 --transmit-mode 0 --payload-data="SGVsbG8gICBTaWRld2FsayE=" \
      --wireless-metadata "Sidewalk={Seq=44}"
      {
         "MessageId": "3fc91dfa-80cb-44c4-89ab-326afe64a4f7"
      }

#. If you run into the following error, ensure your IAM user or role has permissions to send data to your wireless device:

   .. code-block:: console

      {
         "Message": "User: arn:aws:iam::[AWS Account ID]:user/console_user is not authorized to perform:
         iotwireless:SendDataToWirelessDevice on resource: arn:aws:iotwireless:us-east-1:[AWS Account ID]:
         WirelessDevice/[Wireless Device ID]"
      }


.. _sidewalk_cli:

Sidwalk command-line interface (CLI)
************************************

Sidewalk application can be build with the CLI support to help with testing and debugging.
To enable CLI, add the ``CONFIG_SIDEWALK_CLI=y`` option to one of the following places:

* Menuconfig
* Build command
* :file:`prj.conf` file

To verify Sidewalk CLI, open UART shell of the device on the default speed 115200.
Once you see a prompt ``uart:~$``, type the ``sidewalk help`` command to see the avaliable commands.

Currently there are 3 commands avaliable:

- ``sidewalk press_button {1,2,3,4}`` - Simulates button press.
  It can be useful for remote development or for test automation.
- ``sidewalk send <hex payload>`` - Sends message to AWS. The payload have to be hex string without any prefix, and number of characters have to be even.
- ``sidewalk report [--oneline] get state of the application`` - Presents a report in JSON format with the internal state of the application.
- ``sidewalk version [--oneline] print version of sidewalk and its components`` - Presents a report in JSON format with versions of components that build the Sidewalk application.
- ``sidewalk factory_reset perform factory reset of Sidewalk application`` - Performs factory reset.

See the example report output:

.. code-block:: console

   uart:~$ sidewalk report
   "SIDEWALK_CLI": {
         "state": "invalid", 
         "registered": 1, 
         "time_synced": 1, 
         "link_up": 0, 
         "link_modes": {
                  "ble": 0, 
                  "fsk": 0, 
                  "lora": 0
         }, 
         "tx_successfull": 4, 
         "tx_failed": 0, 
         "rx_successfull": 0
   }

See the example version output:

.. code-block:: console

   uart:~$ sidewalk version
   "COMPONENTS_VERSION": {
        "sidewalk_fork_point": "ab13e49adea9edd4456fa7d8271c8840949fde70", 
        "modules": {
                "sidewalk": "v1.12.1-57-gab13e49-dirty", 
                "nrf": "v2.0.0-734-g3904875f6", 
                "zephyr": "v3.0.99-ncs1-4913-gf7b0616202"
        }
   }

.. note::
    By default, only core components are printed.
    To show versions of all components, set ``CONFIG_SIDEWALK_GENERATE_VERSION_MINIMAL`` to ``n`` in file:`prj.conf` file or in the menuconfig.
   
.. _Sidewalk_Handler CloudWatch log group: https://console.aws.amazon.com/cloudwatch/home?region=us-east-1#logsV2:log-groups/log-group/$252Faws$252Flambda$252FSidewalk_Handler
.. _AWS IoT MQTT client: https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html
.. _Installing or updating the latest version of the AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/getting-started-install.html
.. _ID users change permissions: https://docs.aws.amazon.com/IAM/latest/UserGuide/id_users_change-permissions.html
.. _CloudShell: https://console.aws.amazon.com/cloudshell
.. _NCS testing applications: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_testing.html

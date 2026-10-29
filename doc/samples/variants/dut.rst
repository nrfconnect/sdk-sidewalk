.. _variant_sidewalk_dut:

Device Under Test (DUT)
#######################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

Configuration options
*********************

Before building the Device Under Test application variant of the Sidewalk end device, ensure you have enabled the ``OVERLAY_CONFIG="overlay-dut.conf"`` configuration.

.. include:: ../../includes/include_kconfig_common.txt

Building and running
********************

.. include:: ../../includes/include_building_and_running.txt

Testing
=======

#. After successfully building and flashing, the sample is ready to use.
   You should see the command-line prompt ``uart:~$``.
   Run ``sid ?`` to get the list of supported Sidewalk commands.

   .. code-block:: console

       uart:~$ sid ?

   .. note::

      The Sidewalk commands are implemented by the Sidewalk SDK command-line interface.
      Command results (``CMD: ERR: <value> ...``) and Sidewalk events (``EVENT SID ...``) are only printed when the application is built with the ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig option.

#. Initialize Sidewalk in the desired transport mode:

   .. note::

      To use sub-GHz radio, you need to connect radio over SPI, and build the  Sidewalk application with LoRa and FSK library variant.

   a. For Bluetooth LE only mode, run:

      .. code-block:: console

          uart:~$ sid init 1

   #. For sub-GHz radio, run:

      .. code-block:: console

         uart:~$ sid init 7

      Use ``2`` for FSK, ``4`` for Bluetooth LE and LoRa, and ``7`` for all transports.
      For more available options see ``sid init ?``.

#. Start the Sidewalk stack

   .. code-block:: console

      uart:~$ sid start

#. Wait for Sidewalk to be ready (``State: 0``)

   .. code-block:: console

      EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1

#. Send message to cloud.

   You can use `AWS IoT MQTT client`_ to view the received and republished messages from the device.

   #. Enter ``#`` and click :guilabel:`Subscribe to topic`.
      You are now subscribed to the republished device messages.

   #. To see the data republished into the subscribed MQTT topic, use the ``send`` command:

      .. code-block:: console

         uart:~$ sid send test_message
         CMD: ERR: 0 TYPE: 0 ID: 4
         EVENT SID SEND STATUS: SID ERR: 0, TYPE: 0, ID: 4

   You should see the message data in the base64 format:

   .. code-block:: json

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
      }

   You can use ``last_status`` to check if your device is connected to the cloud.
   ``Link_Mask`` must not be ``0``.

   .. code-block:: console

      uart:~$ sid last_status
      EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1
      EVENT SID STATUS LINKS: LoRa: 0, FSK: 0, BLE: 1

   .. _variant_sidewalk_dut_testing_conn_req:

   #. When operating in Bluetooth LE mode, if Sidewalk is not ready, initiate a Bluetooth LE connection every time you need to send a message.

      .. code-block:: console

         EVENT SID STATUS: State: 1, Reg: 0, Time: 0, Link_Mask: 0
         uart:~$ sid conn_req 1
         <inf> sid_ble_conn: BT Connected
         EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1
         uart:~$ sid send test_message

#. Receive message from cloud.

.. include:: ../../includes/include_testing_receiving_message.txt

.. _variant_sidewalk_dut_test_location:

Testing Location Services
==========================

#. Initialize and start a Sidewalk stack (for example, for Bluetooth LE and LoRa support):

   .. code-block:: console

      uart:~$ sid init 4
      uart:~$ sid start

#. Initialize location services:

   .. code-block:: console

      uart:~$ location init

   You should see a log message with a success status.

#. Send the location depending on the protocol used:

   a. For the Bluetooth LE location, check its ``Link status``.
      Send ``conn_req`` if Bluetooth LE link is not ``Up``.

      .. code-block:: console

         uart:~$ sid last_status
         EVENT SID STATUS: State: 1, Reg: 0, Time: 0, Link_Mask: 0
         uart:~$ sid conn_req 1
         <inf> sid_ble_conn: BT Connected
         EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1

      Send location to cloud based on the current Bluetooth LE connection.

      .. code-block:: console

         uart:~$ location send 1

   #. For GNSS location, ensure that the LR1110 radio hardware is connected.
      For Wi-Fi location, use either the LR1110 radio or an nRF70 Series Wi-Fi companion IC, as described in :ref:`location_services`.

      .. note::

         To perform a Wi-Fi or GNSS scan, more time is required.
         It may take up to a few minutes to send location data depending on the signal strength.

      Use ``3`` for Wi-Fi and ``4`` for GNSS.

      .. code-block:: console

         uart:~$ location send 3
         <inf> sidewalk: mw_wifi_scan_add_task: add task in supervisor at 2393 + 0 s
         <inf> sidewalk: mw_wifi_scan_service_on_launch
         <inf> sidewalk: RP: Task #3 enqueue with #11 priority
         <inf> sidewalk: Enqueued RP task for Wi-Fi scan (hook_id #3)
         <inf> sidewalk: mw_wifi_scan_service_on_update
         <inf> sidewalk: sid_pal_hold
         <inf> sidewalk: INFO: Wi-Fi task launch at 2394270
         <inf> sidewalk:
         <inf> sidewalk: INFO: wifi_rp_task_done at 2396937 (duration:2667 ms)
         <inf> sidewalk:
         <inf> sidewalk: Filter and Sort Wi-Fi results:
         <inf> sidewalk: Raw data:
         <inf> sidewalk: 1A 2B 3C 4D 5E 6F  -- Channel: 1 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: AA BB CC 00 11 22  -- Channel: 1 -- Type: 1 -- RSSI: -92 -- Origin: FIXED
         <inf> sidewalk: 01 23 45 67 89 AB  -- Channel: 6 -- Type: 1 -- RSSI: -90 -- Origin: FIXED
         <inf> sidewalk: AA BB CC 00 11 22  -- Channel: 6 -- Type: 1 -- RSSI: -82 -- Origin: UNKNOWN
         <inf> sidewalk: A0 B1 C2 D3 E3 F4  -- Channel: 9 -- Type: 1 -- RSSI: -93 -- Origin: FIXED
         <inf> sidewalk: 11 AA 22 BB 33 DD  -- Channel: 9 -- Type: 1 -- RSSI: -84 -- Origin: FIXED
         <inf> sidewalk: AB 00 CD 11 EF 22  -- Channel: 9 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: A0 B0 C0 D0 E0 F0  -- Channel: 2 -- Type: 2 -- RSSI: -80 -- Origin: UNKNOWN
         <inf> sidewalk:
         <inf> sidewalk: filtered data:
         <inf> sidewalk: 1A 2B 3C 4D 5E 6F  -- Channel: 1 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: AA BB CC 00 11 22  -- Channel: 1 -- Type: 1 -- RSSI: -92 -- Origin: FIXED
         <inf> sidewalk: 01 23 45 67 89 AB  -- Channel: 6 -- Type: 1 -- RSSI: -90 -- Origin: FIXED
         <inf> sidewalk: A0 B1 C2 D3 E3 F4  -- Channel: 9 -- Type: 1 -- RSSI: -93 -- Origin: FIXED
         <inf> sidewalk: 11 AA 22 BB 33 DD  -- Channel: 9 -- Type: 1 -- RSSI: -84 -- Origin: FIXED
         <inf> sidewalk: AB 00 CD 11 EF 22  -- Channel: 9 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk:
         <inf> sidewalk: Sorted data:
         <inf> sidewalk: 11 AA 22 BB 33 DD  -- Channel: 9 -- Type: 1 -- RSSI: -84 -- Origin: FIXED
         <inf> sidewalk: 1A 2B 3C 4D 5E 6F  -- Channel: 1 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: AB 00 CD 11 EF 22  -- Channel: 9 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: 01 23 45 67 89 AB  -- Channel: 6 -- Type: 1 -- RSSI: -90 -- Origin: FIXED
         <inf> sidewalk: AA BB CC 00 11 22  -- Channel: 1 -- Type: 1 -- RSSI: -92 -- Origin: FIXED
         <inf> sidewalk:
         <inf> sidewalk: Event Received: SMTC_MODEM_EVENT_WIFI_SCAN_DONE
         <inf> sidewalk: SCAN_DONE info:
         <inf> sidewalk: -- number of results: 5
         <inf> sidewalk: -- power consumption: 8194 nah
         <inf> sidewalk: -- scan duration: 2905 ms
         <inf> sidewalk: 11 AA 22 BB 33 DD  -- Channel: 9 -- Type: 1 -- RSSI: -84 -- Origin: FIXED
         <inf> sidewalk: 1A 2B 3C 4D 5E 6F  -- Channel: 1 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: AB 00 CD 11 EF 22  -- Channel: 9 -- Type: 1 -- RSSI: -86 -- Origin: FIXED
         <inf> sidewalk: 01 23 45 67 89 AB  -- Channel: 6 -- Type: 1 -- RSSI: -90 -- Origin: FIXED
         <inf> sidewalk: AA BB CC 00 11 22  -- Channel: 1 -- Type: 1 -- RSSI: -92 -- Origin: FIXED
         <inf> sidewalk:
         <inf> sidewalk: Event Received: SMTC_MODEM_EVENT_WIFI_SCAN_DONE
         <inf> sidewalk: SCAN_DONE info:
         <inf> sidewalk: -- number of results: 5
         <inf> sidewalk: -- power consumption: 8353 nah
         <inf> sidewalk: -- scan duration: 2667 ms
         <inf> sidewalk:
         <inf> sidewalk: mw_wifi_send_add_task: add task in supervisor
         <inf> sidewalk: mw_wifi_send_add_task: no scan to be sent
         <inf> sidewalk: Event Received: SMTC_MODEM_EVENT_WIFI_TERMINATED
         <inf> sidewalk: Calling _app_event_lbm_wifi
         <inf> sidewalk: _app_event_lbm_wifi completed
         <inf> sidewalk: rp_hook_callback task WIFI state RP_TASK_STATE_FINISHED
         <inf> sidewalk: sid_pal_release

      If you only need to perform a scan without sending your location to the cloud, you can test your location using the ``scan`` command:

      .. code-block:: console

         uart:~$ location scan 3

   Check the MQTT topic used as Location Destination in `AWS IoT MQTT client`_.
   You should see the location data in the following format:

   .. code-block:: json

      {
         "coordinates": [
            13.37607669,
            52.51823043,
            0
         ],
         "WirelessDeviceId": "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx",
         "type": "Point",
         "properties": {
            "verticalAccuracy": 0,
            "horizontalAccuracy": 100,
            "timestamp": "YYYY-MM-DDTHH:MM:SS.ZZZ"
         }
      }

   If you cannot see the coordinates message, refer to the :ref:`location_services_troubleshooting`.

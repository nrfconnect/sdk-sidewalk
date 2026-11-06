.. _variant_sidewalk_dut:

Device Under Test (DUT)
#######################

.. contents::
   :local:
   :depth: 2

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
   For syntax, options, and shell output formats, see :ref:`variant_sidewalk_dut_commands`.

   .. code-block:: console

       uart:~$ sid ?

   .. note::

      The Sidewalk commands are implemented by the Sidewalk SDK command-line interface.
      Command results (``CMD: ERR: <value> ...``) and Sidewalk events (``EVENT SID ...``) are only printed when the application is built with the ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig option, which is enabled by default when the ``CONFIG_SID_END_DEVICE_CLI`` Kconfig option is enabled, except for the release build type.
      For response and event field descriptions, see :ref:`variant_sidewalk_dut_commands_responses` and :ref:`variant_sidewalk_dut_commands_events`.

#. Initialize Sidewalk in the desired transport mode:

   .. note::

      To use sub-GHz radio, you need to connect radio over SPI, and build the Sidewalk application with LoRa and FSK library variant.

   #. For Bluetooth LE only mode, run:

      .. code-block:: console

          uart:~$ sid init 1

   #. For sub-GHz radio, run:

      .. code-block:: console

         uart:~$ sid init 7

      Use ``2`` for FSK, ``4`` for Bluetooth LE and LoRa, and ``7`` for all transports.
      For the full list of link type and link mask values, see :ref:`variant_sidewalk_dut_commands_link_mask`.
      For more options, run ``sid init ?``.

#. Start the Sidewalk stack:

   .. code-block:: console

      uart:~$ sid start

#. Wait for Sidewalk to be ready (``State: 0``)

   .. code-block:: console

      EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1

#. Send message to cloud.

   You can use `AWS IoT MQTT client`_ to view the received and republished messages from the device.

   #. Enter ``#`` and click :guilabel:`Subscribe to topic`.
      You are now subscribed to the republished device messages.

   #. To see the data republished into the subscribed MQTT topic, use the ``sid send`` command:

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

   You can use ``sid last_status`` to check if your device is connected to the cloud.
   ``Link_Mask`` must not be ``0``.

   .. code-block:: console

      uart:~$ sid last_status
      EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1
      EVENT SID STATUS LINKS: LoRa: 0, FSK: 0, BLE: 1

   .. _variant_sidewalk_dut_testing_conn_req:

   #. When operating in Bluetooth LE mode, if Sidewalk is not ready, initiate a Bluetooth LE connection every time you need to send a message.
      See :ref:`variant_sidewalk_dut_commands_sending_messages` for the ``sid conn_req`` command.

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

For the full ``location`` command syntax, see :ref:`variant_sidewalk_dut_commands_location`.

#. Initialize and start a Sidewalk stack (for example, for Bluetooth LE and LoRa support):

   .. code-block:: console

      uart:~$ sid init 4
      uart:~$ sid start

#. Initialize location services:

   .. code-block:: console

      uart:~$ location init

   You should see a log message with a success status.

#. Send the location depending on the protocol used:

   #. For the Bluetooth LE location, check its ``Link status``.
      Run ``sid conn_req 1`` if the Bluetooth LE link is not up.

      .. code-block:: console

         uart:~$ sid last_status
         EVENT SID STATUS: State: 1, Reg: 0, Time: 0, Link_Mask: 0
         uart:~$ sid conn_req 1
         <inf> sid_ble_conn: BT Connected
         EVENT SID STATUS: State: 0, Reg: 0, Time: 0, Link_Mask: 1

      Send location to cloud based on the current Bluetooth LE connection.

      .. code-block:: console

         uart:~$ location send 1

   #. For FSK gateway location (level L2), re-initialize Sidewalk for FSK only (``sid init 2``), run ``sid start``, and run ``location init`` again.
      Wait until ``sid last_status`` reports ``FSK: Up``.
      Then send location level ``2``:

      .. code-block:: console

         uart:~$ location send 2
         <inf> location_shell_events: location_event_send mode: 2, returned 0

      See :ref:`location_services` for level definitions.

   #. For LoRa multilateration (level L5), re-initialize Sidewalk with a LoRa-capable link mask (``sid init 3`` for LoRa only, or ``sid init 4`` for Bluetooth LE and LoRa; see :ref:`variant_sidewalk_dut_commands_link_mask`), run ``sid start``, and run ``location init`` again.
      Wait until ``sid last_status`` reports ``LoRa: Up``.
      Then send location level ``5``:

      .. code-block:: console

         uart:~$ location send 5
         <inf> location_shell_events: location_event_send mode: 5, returned 0

      Amazon Sidewalk resolves the location only if at least three gateways receive the LoRa uplink.
      For more information, see :ref:`location_services_lora_mlat`.

   #. To try up to six location levels in a specific order, use ``location send_levels``.
      The library tries each level in order and continues to the next if a level is unavailable or does not return a result.
      For example, with Bluetooth LE and LoRa enabled (``sid init 4``), try LoRa multilateration first and fall back to Bluetooth LE gateway location:

      .. code-block:: console

         uart:~$ location send_levels 5 1
         <inf> location_shell_events: location_event_send_levels levels: 2, returned 0

      In the log line above, ``levels: 2`` is the number of levels in the command (not the level value).

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

Testing Registration over Wire
==============================

Registration over Wire (ROW) is an :ref:`experimental <nrf:software_maturity>` feature.
Build the DUT variant of the sample with the ROW overlay:

.. code-block:: console

   $ west build -b nrf54l15dk/nrf54l15/cpuapp -- -DOVERLAY_CONFIG="overlay-dut.conf;overlay-row.conf"

The ``register`` command provides the ``start``, ``get_state``, ``commit_keys`` and ``finalize`` subcommands, which are used by the Amazon Sidewalk manufacturing tool.
Each subcommand replies with a ``{REGISTRATION ...}`` line.
The Registration over Wire stack runs independently of the Sidewalk stack, and the sample does not check that only one of them is in use.
Run ``register start`` only when the Sidewalk stack is not initialized, that is, before ``sid init`` or after ``sid deinit``.
Do not run the ``sid`` commands between ``register start`` and ``register finalize``.

.. _variant_sidewalk_dut_commands:

Command reference
*****************

The DUT variant provides the Sidewalk QA command-line interface (CLI).
It allows testing of the ``sid_api`` functions from the shell and is used to qualify Sidewalk releases.
The shell exposes several command families: ``sid`` (main Sidewalk API), ``location``, and the configuration commands (``print``, ``mac``, ``metrics``, ``tx``, ``time``, and ``tpc``).
Run ``sid ?`` or ``<command> ?`` at the respective root to list subcommands.

In the command descriptions, arguments are shown in angle brackets (``<arg>``) and optional arguments are shown in square brackets (``[<arg>]``).
The Sidewalk APIs are called only when all command arguments are valid.

.. note::

   Before you run any other ``sid`` command, you must run ``sid init`` and ``sid start``.

.. _variant_sidewalk_dut_commands_link_mask:

Link type and link mask values
==============================

Several commands accept a link type or a link mask.
The following table lists the values used by the ``sid init``, ``sid start``, ``sid stop`` and ``sid send -l`` commands.

.. list-table::
   :header-rows: 1
   :widths: 10 90

   * - Value
     - Link type
   * - ``1``
     - ``SID_LINK_TYPE_1`` (Bluetooth LE)
   * - ``2``
     - ``SID_LINK_TYPE_2`` (FSK)
   * - ``3``
     - ``SID_LINK_TYPE_3`` (LoRa)
   * - ``4``
     - ``SID_LINK_TYPE_1 | SID_LINK_TYPE_3`` (Bluetooth LE and LoRa)
   * - ``5``
     - ``SID_LINK_TYPE_1 | SID_LINK_TYPE_2`` (Bluetooth LE and FSK)
   * - ``6``
     - ``SID_LINK_TYPE_2 | SID_LINK_TYPE_3`` (FSK and LoRa)
   * - ``7``
     - ``SID_LINK_TYPE_1 | SID_LINK_TYPE_2 | SID_LINK_TYPE_3`` (Bluetooth LE, FSK and LoRa)
   * - ``8``
     - ``SID_LINK_TYPE_ANY``

.. _variant_sidewalk_dut_commands_stack_control:

Stack control
=============

These commands initialize, start, and stop the Sidewalk stack and query its runtime state.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``sid init <link>``
     - Initializes the Sidewalk stack for the selected link type or link mask.
       Calls ``sid_init()``.
   * - ``sid deinit``
     - Deinitializes the Sidewalk stack.
       Calls ``sid_deinit()``.
   * - ``sid start [<link>]``
     - Starts the Sidewalk stack.
       Calls ``sid_start()``.
       If you omit ``<link>``, the value set with ``sid init`` is used.
   * - ``sid stop [<link>]``
     - Stops the Sidewalk stack.
       Calls ``sid_stop()``.
       If you omit ``<link>``, the value set with ``sid init`` is used.
   * - ``sid factory_reset``
     - Performs a factory reset of the board and deletes all registration status.
       Calls ``sid_set_factory_reset()``.
   * - ``sid last_status``
     - Prints the last status of the Sidewalk library in the same format as the ``EVENT SID STATUS`` event.
       Calls ``sid_get_status()``.
   * - ``sid get_state``
     - Prints the Sidewalk state.
   * - ``sid sdk_config``
     - Prints the Sidewalk SDK configuration values.
   * - ``sid sdk_version``
     - Prints the Sidewalk SDK version.
   * - ``sid mem``
     - Prints the memory information.
   * - ``sid set_battery_level <level>``
     - Sets the battery level in percent (0 to 100).
   * - ``sid set_ota_block_size <size>``
     - Sets the OTA block size.
   * - ``reboot``
     - Reboots the device.
       The reboot function must be passed during the initialization.

.. _variant_sidewalk_dut_commands_init_region:

Initialization and region configuration
=======================================

These commands set stack initialization, operating region, diagnostics, and end-device capabilities.
Run region and initialization configuration commands before ``sid init`` when a setting must apply to the next stack start.

``sid set_init_cfg <metrics> <fsk_ffs> <ffs_retry_period> [<link3_data_rate>]``
   Sets the initialization configuration of the Sidewalk stack.
   Run this command before every ``sid init`` for the new setting to take effect.

   * ``<metrics>`` - Enables or disables sub-GHz metrics reporting to the cloud services.
   * ``<fsk_ffs>`` - Enables or disables Frictionless Setup (FFS) over the sub-GHz FSK link.
     It does not apply to Bluetooth LE.
   * ``<ffs_retry_period>`` - Sets the retry period of FFS over FSK, in seconds.
   * ``<link3_data_rate>`` - Optional.
     Sets the requested LoRa data rate: ``0`` for ``SID_LINK3_DATA_RATE_LDR`` or ``1`` for ``SID_LINK3_DATA_RATE_MDR``.
     If you omit it, the field is unchanged.
     The command is rejected when the Sidewalk stack is initialized.

   The MDR is changed to LDR in regions that do not support it.
   Use ``sid lora_data_rate`` after ``sid init`` to read the applied rate.

``sid region [<region_code>]``
   Gets or sets the operating region.
   The Sidewalk stack must be deinitialized to set the region.
   Without an argument, the command prints the current region.
   Run ``sid init`` after setting the region to apply it.
   The command sets the regulatory information, the radio region and the TX power limits.

   * ``1`` - NA (North America)
   * ``2`` - EU (Europe)

``sid set_region [<region_code>]``
   Gets or sets the device information region used by the region safety gate.
   Without an argument, the command prints the current device information region and whether it is set.
   This value is separate from the operating region set with ``sid region`` and does not configure the radio or the TX power limits.
   The Sidewalk QA CLI initializes the device information region to NA, so registration and sub-GHz operation are not blocked by default.

   * ``1`` - NA
   * ``2`` - EU

``sid set_diag_log_cfg <log_sev> <sdk_log_size> <app_log_size>``
   Sets the configuration of Sidewalk diagnostic logging.
   Run this command before every ``sid init`` for the new setting to take effect.

   * ``<log_sev>`` - Sets ``sid_diag_log_level``: ``0`` (not set, defaults to info), ``1`` (error), ``2`` (info) or ``3`` (debug).
   * ``<sdk_log_size>`` - Sets the size of the log buffer that the DUT application allocates for the SDK logs.
   * ``<app_log_size>`` - Sets the size of the log buffer that the DUT application allocates for the application logs.

``sid ep_cap notify``
   Resends the end-device capabilities notification to the cloud.
   It requires a connected link.

``sid ep_cap dev_type [<type>]``
   Gets or sets the end-device type.
   Run this command before ``sid init``, because the value is read during initialization and a later change is rejected.
   Without an argument, the command prints the current type.

   * ``1`` - Static device (reports ``STATIC_DEVICE``).
   * ``2`` - Mobile device.
     The device is then not eligible for GSK, which is enforced by the gateway and the cloud.

.. _variant_sidewalk_dut_commands_sending_messages:

Sending messages
================

These commands send uplink data and configure related messaging, time sync, and LoRa power-saving behavior.

Uplink send
-----------

The ``sid send`` command sends data over the selected link type and supports optional flood messaging.

``sid send [-t <tv>] [-d <dv>] [-l <lm>] [-i <id>] [-o <low>] [-a <ack> <retry> <ttl>] [-loc] [-f <flood_msg> <flood_period> <delay_sec>] [-r] <data>``
   Sends data over the selected link type.
   Calls ``sid_put_msg()``.
   The data must be the last argument.
   Unless you use the ``-r`` option, the data is treated as an ASCII string.

   The following options are available:

   * ``-t <tv>`` - Sets the message type in the message descriptor:

     * ``0`` - ``SID_MSG_TYPE_GET``
     * ``1`` - ``SID_MSG_TYPE_SET``
     * ``2`` - ``SID_MSG_TYPE_NOTIFY`` (default)
     * ``3`` - ``SID_MSG_TYPE_RESPONSE``

     For the response type, the message ID is the one received with the last GET message.

   * ``-d <dv>`` - Sets the destination: ``1`` for ``SID_LINK_MODE_CLOUD`` (default) or ``2`` for ``SID_LINK_MODE_MOBILE``.
   * ``-l <lm>`` - Sets the link mask on which the message is sent.
     If not set, ``SID_LINK_TYPE_ANY`` is used.
   * ``-i <id>`` - Sets the message ID used to send a response.
     It is valid only for messages of the response type.
   * ``-r`` - Interprets the data as a hexadecimal string, for example ``010203AAFF``.
   * ``-o <low>`` - Sets the low latency configuration: ``0`` to disable (default) or ``1`` to send the message as a LoRa low-latency message.
   * ``-a <ack> <retry> <ttl>`` - Configures the transport ACK:

     * ``<ack>`` - ``1`` to enable or ``0`` to disable the ACK.
     * ``<retry>`` - Number of retries (0 to 255).
     * ``<ttl>`` - Time in seconds for which the stack holds the message in its queue (0 to 65535).

   * ``-loc`` - Attaches a location request (network data format 6) to the message.
     It takes no arguments and works on Bluetooth LE and LoRa links.
   * ``-f <flood_msg> <flood_period> <delay_sec>`` - Configures flood messages:

     * ``<flood_msg>`` - ``0`` stops the flood, ``1`` sends continuous messages without an ACK or response from the Sidewalk network, and ``2`` sends back-to-back uplink messages with ``on_msg_sent`` called after each message.
     * ``<flood_period>`` - Number of messages to send.
     * ``<delay_sec>`` - Delay in seconds applied before the next message after an out-of-memory error.

   Examples:

   .. code-block:: console

      uart:~$ sid send TEST
      uart:~$ sid send -t 0 -d 2 TEST
      uart:~$ sid send -t 1 -d 1 -r 01AAFF02
      uart:~$ sid send -r 0123 -f 1 1000 2
      uart:~$ sid send -r 0123 -f 2 500 3

   The first flood example sends 1000 messages continuously without an ACK.
   If an out-of-memory error occurs for the 501st message, a delay of two seconds is applied and sending continues from message 502.
   The second flood example sends 500 messages with ``on_msg_sent`` called after each message.

Related messaging commands
----------------------------

These commands query message limits, set destination IDs, manage Bluetooth LE connection requests, configure time sync intervals, and control LoRa power saving.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``sid get_mtu <link>``
     - Gets the MTU for the selected link type (``1``, ``2``, ``3`` or ``8`` for any).
       Calls ``sid_get_mtu()``.
   * - ``sid set_dst_id <id>``
     - Sets the message destination ID.
       Calls ``sid_set_msg_dest_id()``.
   * - ``sid conn_req <0,1>``
     - Sets (``1``) or clears (``0``) the connection request bit in the Bluetooth LE beacon.
       Calls ``sid_ble_bcn_connection_request()``.
   * - ``sid get_time <0>``
     - Gets the GPS time (``0``) from the Sidewalk library.
       Calls ``sid_get_time()``.
   * - ``sid set_time_sync_intervals <h1> [<h2> ...]``
     - Sets the time sync intervals array in hours, with up to 10 values.
       For example, ``sid set_time_sync_intervals 2 8 48 96`` triggers a time sync after 2 hours, then 8 hours after the previous sync, then 48 hours, and so on.
   * - ``sid get_time_sync_intervals``
     - Gets the current time sync intervals array.
   * - ``sid set_ps1``
     - Turns on power saving on the LoRa link.
       The LoRa link is automatically delinked and deinitialized.
   * - ``sid set_ps0``
     - Turns off power saving on the LoRa link.
       The LoRa link is automatically initialized and linked, and the time sync is obtained without a cloud request.

Uplink test
-----------

The ``sid uplink_test`` command sends messages of a fixed size at a fixed period for throughput testing.

``sid uplink_test -e <en> -l <lm> -s <size> -n <itr> -p <period> -a <ack> <retry> <ttl>``
   Starts or stops the uplink test and prints the results when stopped.

   * ``<en>`` - ``1`` to start or ``0`` to stop the test and print the results.
   * ``<lm>`` - Link mask (see ``sid send``).
   * ``<size>`` - Size of each message.
   * ``<itr>`` - Total number of messages to send.
   * ``<period>`` - Delay between messages, in seconds.
   * ``<ack>``, ``<retry>`` and ``<ttl>`` - Transport ACK, number of retries and time to live, in seconds, as for ``sid send -a``.

   For example, the ``sid uplink_test -e 1 -l 1 -s 210 -n 10 -p 5 -a 1 3 30`` command starts the test on the Bluetooth LE link.
   It sends 10 messages of 210 bytes every 5 seconds, with ACK enabled, three retries and a time to live of 30 seconds.
   The ``sid uplink_test -e 0`` command prints the results of the test.

.. _variant_sidewalk_dut_commands_link_options:

Link options
============

The ``sid option`` command sets and gets link options.
It calls ``sid_option()``.
The command has the ``sid option <option> [<val1> ... <valN>]`` format.

In the following table, the first column lists the ``<option>`` argument and its parameters.
Run it on the shell as ``sid option <option> ...``, for example ``sid option -lp_set 1`` or ``sid option -lp_get_l2``.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - ``sid option`` argument
     - Description
   * - ``-lp_set 1 [<bcn_int_unit>]``
     - Sets ``SID_LINK2_PROFILE_1``.
       ``<bcn_int_unit>`` is the optional beacon interval unit, from 1 (default) to 3.
       Each unit is 10080 ms.
   * - ``-lp_set 2 <rx_int> [<bcn_int_unit> <rx_duration>]``
     - Sets ``SID_LINK2_PROFILE_2``.
       ``<rx_int>`` is the interval between downlink RX opportunities in milliseconds.
       It must be a multiple of 63 ms and defaults to 63 ms.
       ``<rx_duration>`` is the duration of the RX window opened after an uplink in seconds: ``0`` is infinite (default) and ``65535`` is uplink only.
   * - ``-lp_set 0x80 <rxwc>``, ``-lp_set 0x81 <rxwc>``, ``-lp_set 0x83 <rxwc>``
     - Sets ``SID_LINK3_PROFILE_A``, ``SID_LINK3_PROFILE_B`` or ``SID_LINK3_PROFILE_D``.
       ``<rxwc>`` is the RX window count (``uint8``), where ``0`` means infinite windows.
   * - ``-lp_set 0x82``
     - Sets ``SID_LINK3_PROFILE_C``.
   * - ``-lp_get_l2``, ``-lp_get_l3``
     - Gets the link profile and its parameters for ``SID_LINK_TYPE_2`` or ``SID_LINK_TYPE_3``.
   * - ``-sub_ghz_ctl <ctl_cmd>``
     - Sends a user control command on sub-GHz links.
       ``1`` temporarily stops the RX window for ``SID_LINK2_PROFILE_2`` or ``SID_LINK3_PROFILE_A``.
       The RX window opens again after the next successful uplink.
   * - ``-l3_tpc_cfg <enabled>``
     - Enables (``1``) or disables (``0``) transmit power control on ``SID_LINK_TYPE_3``.
       It requires link type 3 to be initialized.
       The default configuration is used when link type 3 is initialized again.
   * - ``-d <0,1>``, ``-gd``
     - Sets (``-d``) or gets (``-gd``) the duplicate message filter.
       ``0`` filters duplicate messages and ``1`` passes them to the user.
   * - ``-m <policy>``, ``-gm``
     - Sets (``-m``) or gets (``-gm``) the link connection policy.
       ``0`` is ``SID_LINK_CONNECTION_POLICY_NONE``, ``1`` is ``SID_LINK_CONNECTION_POLICY_AUTO_CONNECT`` and ``2`` is ``SID_LINK_CONNECTION_POLICY_MLM``.
   * - ``-c <link> <enable> <priority> <timeout>``, ``-gc <link>``
     - Sets (``-c``) or gets (``-gc``) the auto-connect policy for a link (``1``, ``2`` or ``3``).
       ``<enable>`` is ``0`` or ``1``.
       ``<priority>`` is from ``0`` (highest) to ``255`` (lowest).
       ``<timeout>`` is the total time in seconds for which the stack tries to establish a connection.
       ``<priority>`` and ``<timeout>`` are optional when you disable auto-connect.
   * - ``-ml <policy>``, ``-gml``
     - Sets (``-ml``) or gets (``-gml``) the multilink policy.
       ``0`` is default, ``1`` is power save, ``2`` is performance, ``3`` is latency and ``4`` is reliability (``SID_LINK_MULTI_LINK_POLICY_*``).
   * - ``-st_get``, ``-st_clear``
     - Gets or clears the statistics (TX, ACKs sent, TX failures, retries, duplicates, ACKs received and RX).
   * - ``-gsi``
     - Gets the Sidewalk ID.
   * - ``-da_cfg <cmd> [<delayed_ack_time>]``
     - Gets (``0``) or sets (``1``) the delayed ACK time in milliseconds, from ``0`` to ``20000``.
   * - ``-dlrv_cfg <is_set> <dlrv_time> [<nwsync_time>]``
     - Gets (``0``) or sets (``1``) the downlink route validity time.
       ``<dlrv_time>`` is in minutes, from 1 to 1440 (the FSK default is 1080).
       ``<nwsync_time>`` is the keep-alive (network sync) interval in minutes: 0 to 1440 for FSK and 0 to 240 for LoRa Profile B.
       ``0`` means that the link default is used.
       The value is applied to every enabled sub-GHz link.
   * - ``-ble_d2d_only <cmd> <enable>``
     - Gets (``0``) or sets (``1``) the Bluetooth LE D2D state through ``sid_option()``.
       ``<enable>`` is ``0`` to disable or ``1`` to enable D2D.
       The ``sid ble_d2d_config`` shell command is an equivalent wrapper around this option.
   * - ``-ble_con <is_set> <conn_policy> <policy_param>``
     - Gets (``0``) or sets (``1``) the Bluetooth LE connection policy.
       ``<conn_policy>`` is ``0`` (default), ``1`` (long-lived connection) or ``2`` (optimal advertising).
       ``<policy_param>`` is required only for the long-lived connection policy: ``0`` optimizes for power and ``1`` optimizes for latency.

The following ``sid option`` arguments have additional parameters beyond the table.

``-ble_cfg <cmd> <type> <v1> ... <v8>``
   Gets or sets the Bluetooth LE configuration.

   * ``<cmd>`` - ``0`` to get or ``1`` to set.
   * ``<type>`` - ``0`` for the advertising configuration, ``1`` for the connection configuration, ``2`` for both, or ``3`` for the inactivity timeout.
   * ``<v1>`` - The inactivity timeout in seconds, required for the inactivity timeout configuration.
   * ``<v1>`` to ``<v4>`` - Required for the advertising or connection configuration.
   * ``<v1>`` to ``<v8>`` - Required for the combined configuration.
     The first four values are the advertising parameters and the last four are the connection parameters.

   The advertising parameters are:

   * ``<fast_interval>`` - Fast advertising interval in units of 0.625 ms, from 16 to 8192.
   * ``<fast_timeout>`` - Fast advertising timeout in units of 10 ms, from 0 to 65535, where ``0`` is indefinite.
   * ``<slow_interval>`` - Slow advertising interval in units of 0.625 ms, from 800 to 8192.
   * ``<slow_timeout>`` - Slow advertising timeout in units of 10 ms, from 0 to 65535, where ``0`` is indefinite.

   The connection parameters are:

   * ``<min_int>`` - Minimum connection interval in units of 1.25 ms, from 0 to 3200.
   * ``<max_int>`` - Maximum connection interval in units of 1.25 ms, from 0 to 3200.
   * ``<sl>`` - Peripheral latency, from 0 to 499.
   * ``<sup_timeout>`` - Supervision timeout in units of 10 ms, up to 3200 (32 s).

   The connection parameters must satisfy ``min_int <= max_int`` and ``max_int * 1.25 * (sl + 1) < sup_timeout``.

``-gwd_cfg <cmd> [<policy>] [<scan> <cont> <rssi_conn> <rssi_disc> <rssi_num> <bcn_miss> <bcn_skip> <use_load> <load_disc> <use_health> <use_rank>]``
   Gets or sets the gateway discovery policy parameters.
   This option applies to FSK only.

   * ``<cmd>`` - ``0`` to get the policy.
     For the custom policy, the command also prints its parameters and backoff parameters.
   * ``<cmd>`` - ``1`` to set the policy.
   * ``<policy>`` - ``0`` for default, ``1`` for fast connection, ``2`` for reliable connection, ``3`` for power save, or ``4`` for custom (``SID_LINK_TYPE_2_GW_DISCOVERY_POLICY_*``).

   The custom policy requires the following parameters:

   * ``<scan>`` - Gateway discovery scanning timeout in seconds.
   * ``<cont>`` - ``1`` to continue scanning after the scanning timeout expires, ``0`` to disable.
   * ``<rssi_conn>`` - Minimum RSSI at which a connection is allowed, where ``0`` means not used.
   * ``<rssi_disc>`` - RSSI at which the end device disconnects from the gateway.
   * ``<rssi_num>`` - Number of beacons used to calculate the average RSSI, where ``0`` means not used.
   * ``<bcn_miss>`` - Maximum number of consecutive missed beacons before the end device reports a disconnection.
   * ``<bcn_skip>`` - Maximum number of beacons the end device can skip listening to.
   * ``<use_load>`` - ``1`` to use the gateway load to calculate the gateway score, ``0`` to disable.
   * ``<load_disc>`` - Gateway load at which the end device disconnects, from 0 to 3, where ``0`` means not used.
   * ``<use_health>`` - ``1`` to use the gateway connection health to calculate the gateway score, ``0`` to disable.
   * ``<use_rank>`` - ``1`` to use the gateway transmit power rank to calculate the gateway score, ``0`` to disable.

``-gwd_cfg 3 <backoff_size> <threshold_1> <scan_1> <sleep_1> [<threshold_2> <scan_2> <sleep_2> ...]``
   Presets the backoff parameters of the custom gateway discovery policy (FSK only).
   Run this command before you set the custom policy.

   * ``<backoff_size>`` - Size of the backoff parameters array, from 1 to 4.
   * ``<threshold_N>`` - Retry threshold at which ``<scan_N>`` and ``<sleep_N>`` are applied, from 1 to 255, where ``255`` means infinity.
   * ``<scan_N>`` - Scan duration in seconds, from 1 to 65535.
   * ``<sleep_N>`` - Sleep time between scans in seconds, from 0 to 65535.

.. _variant_sidewalk_dut_commands_link_profile_examples:

Device profile examples
-----------------------

The following examples show how to set and get the device profile.

.. code-block:: console

   uart:~$ sid option -lp_set 1
   uart:~$ sid option -lp_get_l2
   <inf> app: CMD: ERR: 0 Link_profile ID: 1 Wndw_cnt: 0
   uart:~$ sid option -lp_set 2 5040
   uart:~$ sid option -lp_get_l2
   <inf> app: CMD: ERR: 0 Link_profile ID: 2 Wndw_cnt: 0 Rx_Int = 5040
   uart:~$ sid option -lp_set 128 5 1
   uart:~$ sid option -lp_get_l3
   <inf> app: CMD: ERR: 0 Link_profile ID: 128 Wndw_cnt: 5 Low_Latency = 1

The first example sets ``SID_LINK2_PROFILE_1``.
The second example sets ``SID_LINK2_PROFILE_2`` with a downlink interval of 5040 ms between RX opportunities.
The third example sets ``SID_LINK3_PROFILE_A`` with five RX windows and low latency enabled.

.. _variant_sidewalk_dut_commands_location:

Location commands
=================

The ``location`` commands initialize the Sidewalk location module and send location data.
For the location testing procedure, see :ref:`variant_sidewalk_dut_test_location`.

The location commands use the following effort levels of the Sidewalk SDK 2.0:

.. list-table::
   :header-rows: 1
   :widths: 10 45 45

   * - Level
     - Effort level
     - Required hardware or link
   * - ``1``
     - L1: BLE gateway
     - Bluetooth LE link
   * - ``2``
     - L2: FSK gateway
     - FSK link
   * - ``3``
     - L3: Wi-Fi scan
     - LR1110 radio or nRF7002 EB companion IC
   * - ``4``
     - L4: GNSS scan
     - LR1110 radio
   * - ``5``
     - L5: LoRa multilateration
     - LR1110 or SX1262 radio, LoRa link
   * - ``6``
     - L6: GPS assist
     - Not supported

For more information about the levels, see :ref:`location_services`.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``location init``
     - Initializes the Sidewalk location library.
   * - ``location deinit``
     - Deinitializes the Sidewalk location library.
   * - ``location send [<level>]``
     - Sends a location uplink at the specified level.
       Without a level, the library selects the level automatically, starting from the lowest one.
       When a scan fails, for example when the Wi-Fi scan returns no results, the library moves to a more capable level.
       After a successful scan at a higher level, the step-down timers gradually reduce the level, so that the next request tries the lower level again.
       The step-down timeouts are configured in ``sid_location_config.stepdowns`` (``l4_to_l3`` and ``l3_to_l2``).
   * - ``location send_levels <L> [<L> ...]``
     - Sends a location uplink using the given order of location levels (up to six levels).
       The library tries the levels in this order and moves to the next one when a level is unsupported or returns insufficient results.
       It overrides the single-level and automatic behavior of ``location send``.
       For example, ``location send_levels 5 1`` tries LoRa multilateration and then Bluetooth LE gateway location.
   * - ``location send_buf <level>``
     - Sends a 50-byte test buffer over an available link without a new scan.
       The buffer does not contain a valid location.
       The level selects the payload type: ``3`` for Wi-Fi or ``4`` for GNSS.
   * - ``location send_pseudorange <hex>``
     - Sends a raw GPS-assist pseudorange payload (compact format 0) as a GNSS notification to the SLS service over the first available link.
       ``<hex>`` is an even-length hexadecimal string of up to 106 bytes, sent without parsing or repacking.
       It requires ``location init`` and the network time synchronization.
       For example, ``location send_pseudorange 20aabbccddee``.
   * - ``location scan <level>``
     - Performs a location scan without sending the result: ``3`` for Wi-Fi or ``4`` for GNSS.
   * - ``location alm_start``
     - Starts the almanac demodulation service to update the GNSS almanac through a satellite.
       The service cannot be stopped.
   * - ``location manage_effort [<0|1>]``
     - Gets or sets the ``manage_effort`` flag in the location configuration.
       ``0`` disables the automatic effort escalation, so a Wi-Fi scan failure does not escalate to GNSS.
       ``1`` enables it (default).
       Without an argument, the command prints the current value.

.. _variant_sidewalk_dut_commands_config:

Sidewalk configuration commands
===============================

The DUT variant also provides the Sidewalk configuration CLI, which consists of the ``print``, ``mac``, ``metrics``, ``tx``, ``time`` and ``tpc`` root commands.
Run ``<command> ?`` to list the subcommands of a root command.

.. note::

   Some parameters apply only to the gateway (GW) or only to the end node (EN).
   This section describes the end node usage.

.. _variant_sidewalk_dut_commands_print:

Print commands
--------------

The ``print`` command prints values from the device.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``print fwversion``
     - Prints the firmware version.
   * - ``print appversion``
     - Prints the application version.
   * - ``print mfg``
     - Prints the manufacturing values.
   * - ``print metrics <category_id>``
     - Prints the metrics for the given category.
       The categories are defined in ``sid_metrics_defines.h``.
   * - ``print clear_metrics <category_id> <metrics_id>``
     - Clears the metrics for the given category.
       The categories are defined in ``sid_metrics_defines.h``.
   * - ``print txid``
     - Prints the obfuscated device ID (TX-ID).
   * - ``print get_cap <value>``
     - Prints the current capability (``1``), the current configuration (``2``), or both (``3``).
   * - ``print clear_cap <value>``
     - Clears the current capability (``1``), the current configuration (``2``), or both (``3``).

.. _variant_sidewalk_dut_commands_mac:

MAC commands
------------

The ``mac`` command configures the MAC layer.

``mac gcfg``
   Gets the MAC configuration parameters.

``mac scfg <parameter> <value> [<value>]``
   Sets a MAC configuration parameter.
   The following parameters are available:

   * ``N`` - Group number, for example ``mac scfg N 5``.
   * ``P`` - PAN ID, for example ``mac scfg P 0xA4C5387B00``.
   * ``I`` - Device ID, for example ``mac scfg I 0xA00008EAAF``.
   * ``R`` - LDR channel.
     Use one value for a single channel (``mac scfg R 0``) or two values for the uplink and downlink channels (``mac scfg R 10 0``).
   * ``H`` - Region code: ``1`` for ``REGION_US915`` or ``2`` for ``REGION_EU868``.
   * ``M`` - Maximum number of missed beacons, for example ``mac scfg M 3``.
   * ``J`` - Pairing state status: ``1`` to enable or ``0`` to disable.
   * ``Q`` - Home PAN scan interval in seconds, up to 65535.
   * ``E`` - Group periodicity, from 1 to 10.
   * ``V`` - Unicast periodicity, from 1 to 10.
   * ``O`` - Group offset, from 0 to 9.
   * ``W`` - Unicast offset, from 0 to 9.
   * ``G`` - Group wakeup schedule.
   * ``D`` - Unicast wakeup schedule.
     For both wakeup schedules, ``0`` disables TX and RX, ``1`` enables TX only, ``2`` enables RX only and ``3`` enables TX and RX.
   * ``C`` - Forces the FSK end node to synchronize to the selected gateway.
     Use ``mac scfg C 1 0xA00008EAAF`` to start and ``mac scfg C 0`` to stop.
   * ``T`` - Continuous FSK TX slot: ``1`` to enable or ``0`` to disable.
   * ``L`` - Low latency parameters in the ``<enabled> <latency> <rate_limit> <repetitions>`` format, for example ``mac scfg L 1 2 4 0``.

``mac skey <key> <value>``
   Sets a security key.
   The value is a 16-byte hexadecimal number, for example ``0x00112233445566778899AABBCCDDEEFF``.
   Use ``PM`` for the PAN master key and ``WM`` for the WAN master key.

``mac rcfg``
   Resets the MAC configuration parameters.
   The device performs a factory reset and all configuration values are reset to the factory defaults.

``mac log <module> <enable> [<log_level>]``
   Sets the log level for a MAC submodule.
   The default log level is ``2``.
   Set ``<enable>`` to ``1`` to enable the logs.
   The ``<module>`` value selects the logs: ``0`` for beacon, ``1`` for probe, ``2`` for HDR, ``3`` for LDR and ``5`` for event list.

.. _variant_sidewalk_dut_commands_metrics:

Metrics commands
----------------

The ``metrics`` command sets and sends the metrics data.

``metrics set <category_id> <metric_id> <value>``
   Sets the value of a ``metrics_fwk_v2`` metric.
   The categories and metric IDs are defined in ``sid_metrics_defines.h``.

``metrics config_set <action> <category_id> <metric_id>``
   Sets the configuration in the ``metrics_fwk_v2`` bitmask.
   The actions are defined in ``sid_metrics_core_ifc.h``.
   The categories and metric IDs are defined in ``sid_metrics_defines.h``.

``metrics send``
   Triggers the default flow of the periodic metrics collection and transmission to the cloud.

.. _variant_sidewalk_dut_commands_tx:

TX commands
-----------

The ``tx`` command transmits with the radio.

``tx rnet <1|0>``
   Sends the command based on the ``rnet_app`` frame settings (``1``), or clears the command (``0``).

``tx rnet_app``
   Forms the RingNet application layer command.
   The following parameters are available:

   * ``A`` - Destination format: ``0`` for cloud, ``1`` for device ID or ``3`` for group ID.
   * ``B`` - Source format.
   * ``C`` - ACK request: ``1`` for yes, ``0`` for no.
   * ``D`` - Destination device ID (five hexadecimal bytes, for example ``0xBFFFFF60BA``).
   * ``E`` - Authentication code.
   * ``F`` - Encryption enabled: ``1`` for yes, ``0`` for no.
   * ``G`` - Response request: ``1`` for yes, ``0`` for no.
   * ``H`` and ``J`` - Sequence number.
   * ``I`` - Security enabled: ``1`` for yes, ``0`` for no.
   * ``K`` - Link for TX: ``1`` for application, ``2`` for radio, ``3`` for Bluetooth LE, ``4`` for SRL or ``5`` for stack.
   * ``L`` - Number of retries.
   * ``M`` - Explicit data length.
   * ``N`` - Suppress broadcast ACK: ``1`` for yes, ``0`` for no.
   * ``S`` - Source device ID.
   * ``T`` - ID.
   * ``U`` - Class ID.
   * ``V`` - Flip data.
   * ``W`` - Data header.
   * ``X`` - Status code.
   * ``Y`` - Data length.
   * ``Z`` - Protocol version.
   * ``a`` - Sleepy device.

``tx dcr <duty_cycle>``
   Sets the duty cycle from 1 to 100 percent.
   The ``tx dcr 0`` command is rejected with the ``0% is invalid, range 1~100%`` error, so it cannot be used to read the current setting.

.. _variant_sidewalk_dut_commands_time:

Time commands
-------------

The ``time`` command sends and sets the time sync command and its parameters.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Command
     - Description
   * - ``time gcs_param [<time_out> <cld_sync_interval> <max_retry>]``
     - Sets the GCS parameters to synchronize (timeout and cloud sync interval in seconds, and maximum number of retries).
       Without arguments, the command gets the GCS parameters.
   * - ``time get_time [<devid>]``
     - Sends ``GET_TIME`` to the given device, or to the cloud if you omit the device ID.
   * - ``time set_time <tv_sec> <drift_ms>``
     - Sets the system time.
   * - ``time now``
     - Prints the current time.
   * - ``time set_offset <offset_ms> <drift_offset_ms>``
     - Sets the time offset.

.. _variant_sidewalk_dut_commands_tpc:

Transmit Power Control commands
-------------------------------

The ``tpc`` command configures the Transmit Power Control (TPC).

``tpc lora scfg [-p <trial_check_period_min> <trial_probe_period_min> <cooldown_period_min>] <enabled>``
   Sets the LoRa TPC configuration.

   * ``<enabled>`` - ``1`` to enable or ``0`` to disable the TPC.
   * ``-p`` - Configures the period parameters in minutes: the minimum check period and the minimum probe period in the trial phase, and the cooldown period that avoids probing with a reduced TX power.
     The values must satisfy ``trial_check_period_min < trial_probe_period_min < cooldown_period_min``.

``tpc lora gcfg``
   Gets the LoRa TPC configuration parameters.

``tpc lora status``
   Gets the current LoRa TPC status.

``tpc fsk scfg <enabled> [<retry_pkt_thres> <retry_pwr_thres> <force_tx_pwr>]``
   Sets the FSK TPC configuration for the end node.
   On a gateway, the command takes ``<enabled> [<minimum_snr>]`` instead.

   * ``<enabled>`` - ``2`` to enable the TPC without the gateway TPC, ``1`` to enable all, or ``0`` to disable.
   * ``<retry_pkt_thres>`` - Packet counter threshold that triggers the retry adjustment of the end node power.
   * ``<retry_pwr_thres>`` - Power threshold that triggers the retry adjustment of the end node power.
   * ``<force_tx_pwr>`` - Forces the end node TX power, from 1 to 22.
     A value of ``0`` or less disables the forced power.

``tpc fsk gcfg``
   Gets the FSK TPC configuration parameters.

``tpc fsk status``
   Gets the current FSK TPC status.

.. _variant_sidewalk_dut_commands_other:

Other commands
--------------

These commands are available from the shell but are not part of the ``sid`` or configuration CLI trees.

``nordic_dfu``
   Starts the Bluetooth LE SMP service for DFU.
   Deinitialize the Sidewalk stack before you use this command, or the DFU might fail during the transfer.

``help``
   Prints the list of commands.

.. _variant_sidewalk_dut_commands_responses:

Command responses
=================

Each command prints the return value of the called Sidewalk API function and the result of its execution in the following format:

.. code-block:: console

   CMD: ERR: <value>

The ``<value>`` is the ``sid_error_t`` value returned by the Sidewalk API.

Some commands print additional information returned by the Sidewalk API:

.. code-block:: console

   uart:~$ sid get_mtu 1
   CMD: ERR: 0 MTU: 255
   uart:~$ sid send Test
   CMD: ERR: 0 TYPE: 0 ID: 4
   uart:~$ sid get_time 0
   CMD: ERR: 0 SEC: 1308486906 NSEC: 857635511

In the ``sid send`` response, ``TYPE`` is the message type and ``ID`` is the ID associated with the message.
In the ``sid get_time`` response, ``SEC`` is the number of seconds and ``NSEC`` is the number of nanoseconds.

.. _variant_sidewalk_dut_commands_events:

Sidewalk events
===============

The DUT application prints an event when one of the Sidewalk callbacks is invoked.

``EVENT SID STATUS``
   Printed when the Sidewalk library state changes.
   It is invoked by the ``on_status_changed()`` callback.

   .. code-block:: console

      EVENT SID STATUS: State: 1, Reg: 1, Time: 1, Link_Mask: 1

   * ``State`` - State of the Sidewalk library: ``0`` for ready, ``1`` for not ready or ``2`` for error.
   * ``Reg`` - Registration status: ``0`` for registered or ``1`` for not registered.
   * ``Time`` - Time synchronization status: ``0`` for synchronized or ``1`` for not synchronized.
   * ``Link_Mask`` - Bit field that describes which links are up.
     For example, to check if ``SID_LINK_TYPE_1`` is up, ``Link_Mask & SID_LINK_TYPE_1`` must not be ``0``.

``EVENT SID STATUS LINKS``
   Printed when the Sidewalk library state changes.
   It expands the ``Link_Mask`` bit field to show which link types are available.

   .. code-block:: console

      EVENT SID STATUS LINKS: LoRa: 1, FSK: 0, BLE: 1
      EVENT SID STATUS LINK 0 MODE 2
      EVENT SID STATUS LINK 1 MODE 0
      EVENT SID STATUS LINK 2 MODE 1

   * ``LoRa``, ``FSK`` and ``BLE`` - Whether the link is available (``1``) or not (``0``).
   * ``LINK`` - Link number: ``0`` for Bluetooth LE, ``1`` for FSK or ``2`` for LoRa.
   * ``MODE`` - Mode supported on the link: ``0`` for not available, ``1`` for cloud destination, ``2`` for mobile destination or ``3`` for cloud and mobile destination.

``EVENT SID SEND STATUS``
   Printed when a message is sent to the network or when a send error occurs.
   It is invoked by the ``on_msg_sent()`` callback.

   .. code-block:: console

      EVENT SID SEND STATUS: SID ERR: 0, TYPE: 0, ID: 4

   * ``SID ERR`` - Sidewalk error.
   * ``TYPE`` - Message type.
   * ``ID`` - ID associated with the message.

``EVENT SID RECEIVED``
   Printed when a message is received from the network.
   It is invoked by the ``on_msg_received()`` callback.

   .. code-block:: console

      EVENT SID RECEIVED: TYPE: 0, ID: 2, LEN: 5
      Data: 0102030405

   * ``TYPE`` - Message type.
   * ``ID`` - ID associated with the message.
   * ``LEN`` - Length of the message payload in bytes.
   * ``Data`` - Payload in hexadecimal format.

``EVENT SID FACTORY RESET``
   Printed when the library performs a factory reset.
   It is invoked by the ``on_factory_reset()`` callback.

``EVENT KA ACK``
   Printed when a KA ACK status is received or when it times out.
   It is invoked by ``on_control_event_notify()`` with ``SID_CONTROL_EVENT_KA_ACK_STATUS``.

   .. code-block:: console

      EVENT KA ACK: link=2 ack=1

   * ``link`` - Link type on which the KA was sent: ``2`` for FSK or ``3`` for LoRa.
   * ``ack`` - ``1`` when the ACK was received from the cloud or ``0`` on timeout.

``EVENT SID RESET``
   Printed after the device boots up.
   It is generated by the DUT application, not by the Sidewalk library, and is required by the test framework.

.. _sidewalk_testing_with_cli:

Testing Sidewalk with CLI
#########################

Sidewalk application can be build with the CLI support to help with testing and debugging.

.. _sidewalk_cli:

Enabling and verifying Sidewalk command-line interface (CLI)
************************************************************

#. To enable CLI, add the ``CONFIG_SIDEWALK_CLI=y`` option to one of the following places:

   * Menuconfig
   * Build command
   * :file:`prj.conf` file

#. To verify Sidewalk CLI, open UART shell of the device on the default speed 115200.
#. Once you see a prompt ``uart:~$``, type the ``sidewalk help`` command to see the avaliable commands.

   Currently there are 3 commands avaliable:

   * ``sidewalk press_button {1,2,3,4}`` - Simulates button press.
     It can be used for remote development or for test automation.
   * ``sidewalk send <hex payload>`` - Sends message to AWS.
     The payload has to be a hex string without any prefix and the number of characters has to be even.
   * ``sidewalk report [--oneline] get state of the application`` - Presents a report in JSON format with the internal state of the application.
   * ``sidewalk version [--oneline] print version of sidewalk and its components`` - Presents a report in JSON format with versions of components that build the Sidewalk application.
   * ``sidewalk factory_reset perform factory reset of Sidewalk application`` - Performs factory reset.

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
    To show versions of all components, set ``CONFIG_SIDEWALK_GENERATE_VERSION_MINIMAL`` to ``n`` in :file:`prj.conf` file or in the menuconfig.

.. _Sidewalk_Handler CloudWatch log group: https://console.aws.amazon.com/cloudwatch/home?region=us-east-1#logsV2:log-groups/log-group/$252Faws$252Flambda$252FSidewalk_Handler
.. _AWS IoT MQTT client: https://docs.aws.amazon.com/iot/latest/developerguide/view-mqtt-messages.html
.. _CloudShell: https://console.aws.amazon.com/cloudshell
.. _NCS testing applications: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_testing.html

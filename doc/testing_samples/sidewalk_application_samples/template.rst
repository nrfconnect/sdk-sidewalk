.. _sidewalk_template:

Sidewalk Template
#################

This sample demonstrates a template for the Sidewalk End Node application.
It supports Bluetooth LE, LoRa, and FSK link modes.
External QSPI Flash is used for firmware updates.

Overview
********

The Sidewalk status is indicated by LEDs state and is also printed in the device logs.
It supports actions, such as sending messages, performing factory reset, and entering the DFU state.
The sample echoes received Sidewalk messages of type GET and SET to the Sidewalk cloud.

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

Configuration
*************

When running the sample, you can use different configuration files depending on the supported features.

.. _sidewalk_template_source_files:

Source file setup
=================

The application consists of two source files:

* :file:`app.c` -- The main application file.
  It implements Sidewalk callbacks, assigns button actions, and initializes components.
* :file:`sidewalk.c` -- Sidewalk thread file.
  It implements a separate thread where the Sidewalk API is called to start, stop, process, switch link modes, and send messages.
  The file also includes the support for the DFU state.
  See the behavior of the state machine outlined in the following diagram:

.. uml::
   :caption: Sidewalk thread state machine

   [*] --> Sidewalk
   state Common{
   Sidewalk --> [*] : event_factory_reset
   Sidewalk --> FirmwareUpdate : event_dfu
   FirmwareUpdate --> Common : event_dfu

.. _sidewalk_template_hello_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

.. include:: testing_sample_include.txt

.. _SX1262: https://os.mbed.com/components/SX126xMB2xAS/

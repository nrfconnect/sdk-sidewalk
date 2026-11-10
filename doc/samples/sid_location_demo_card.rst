.. _sidewalk_location_demo_card:

Sidewalk location demo
######################

.. contents::
   :local:
   :depth: 2

This page describes how to present the :ref:`sidewalk_location` sample to customers.

Overview
********

The demo shows the following features:

* The device gets its position over Amazon Sidewalk, without a SIM card, Wi-Fi credentials, or a phone.
* The device sends its position through a Sidewalk gateway, for example an Echo device, every minute.
* The nRF54LM20 DK with the nRF Sidewalk EB adds Wi-Fi and GNSS location, and the LoRa link.
* Between positions, the device sleeps.

Requirements
************

The demo requires the following:

* An nRF54L15 TAG or an nRF54LM20 DK with the nRF Sidewalk EB, programmed and provisioned as described in :ref:`sidewalk_location`.
* A Sidewalk gateway with Sidewalk enabled.
* A computer with the viewer from :file:`tools/location_demo` and AWS credentials for the demo account.

Preparing the demo
******************

Before the meeting, complete the following steps:

1. Log in to AWS:

   .. code-block:: console

      aws sso login --profile <profile>

#. Start the viewer and open ``http://127.0.0.1:8000`` in a browser:

   .. code-block:: console

      python3 viewer.py --profile <profile>

#. Power on the devices near the gateway and wait for the first position on the map.

Running the demo
****************

Complete the following steps:

1. Show the last position of each device on the map.
#. Explain the periodic location.
   The device tries Bluetooth LE first, and Wi-Fi and GNSS only when Bluetooth LE location fails.
   The LED shows the method: blue or **LED 0** for Bluetooth LE, **LED 1** for Wi-Fi, and **LED 2** for GNSS.
   GNSS needs open sky.
#. Press the button on the TAG or **Button 0** on the DK.
   The device sends a message, and the cloud resolves its position from the gateway.
   On the map, this position has a black outline, and its popup shows the message.

Key points
**********

* Sidewalk is a shared network of Amazon devices that customers already own, with no connectivity fee.
* The device is provisioned once and needs no pairing or app on site.
* Bluetooth LE location uses the least energy.
* Nordic Semiconductor provides the SoC, the Amazon Sidewalk Add-on for the nRF Connect SDK, and this sample.

Troubleshooting
***************

The red LED or **LED 3** blinks once at power-on
   The device is not provisioned.

No new positions appear on the map
   Check that the gateway has Sidewalk enabled.

The map does not load
   Log in to AWS again and restart the viewer.

Privacy and security
********************

* The viewer runs only on the local computer.
* With the read-only demo policy from :file:`tools/location_demo`, the viewer can read only the positions of the demo devices.
* OpenStreetMap receives the map tile requests, but no device data.

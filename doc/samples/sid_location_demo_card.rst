.. _sidewalk_location_demo_card:

Sidewalk location demo card
###########################

One-page guide for the :ref:`sidewalk_location` customer demo.

What it shows
*************

* The device gets its position over Amazon Sidewalk, without a SIM card, Wi-Fi credentials, or a phone.
* Every minute, the device sends its position through a Sidewalk gateway, for example an Echo device.
* The nRF54LM20 DK with the nRF Sidewalk EB adds Wi-Fi and GNSS location, and the LoRa link.
* Between positions, the device sleeps.

What you need
*************

* A provisioned nRF54L15 TAG or nRF54LM20 DK with the nRF Sidewalk EB.
* A Sidewalk gateway with Sidewalk enabled, in a country where Sidewalk is available.
* A laptop with the viewer from :file:`tools/location_demo` and an AWS SSO login.

Before the meeting
******************

1. Run ``aws sso login --profile sidewalk-demo``.
2. Run ``python3 viewer.py --profile sidewalk-demo`` and open ``http://127.0.0.1:8000``.
3. Power on the devices near the gateway and wait for the first position on the map.

Running the demo
****************

1. Show the last position of each device on the map.
2. Explain the periodic location: the device tries Bluetooth LE first, and Wi-Fi and GNSS only when needed.
   The LED shows the method: blue or **LED 0** for Bluetooth LE, **LED 1** for Wi-Fi, **LED 2** for GNSS.
   GNSS needs open sky.
3. Press the TAG button or **Button 0** on the DK.
   The device sends a message, and the cloud resolves its position from the gateway.
   On the map, this position has a black outline, and its popup shows the message.

Key messages
************

* Sidewalk is a free, shared network of Amazon devices that customers already own.
* The device is provisioned once and needs no pairing or app on site.
* Bluetooth LE location uses the least energy.
* Nordic provides the SoC, the Sidewalk Add-on for the nRF Connect SDK, and this sample.

Troubleshooting
***************

* One red blink at power-on: the device is not provisioned.
* No new positions: check that the gateway has Sidewalk enabled.
* The map does not load: run ``aws sso login`` again and restart the viewer.

Privacy and security
********************

* The map runs only on the laptop.
* The viewer has read-only access to the demo devices.
* OpenStreetMap receives the map tile requests, but no device data.

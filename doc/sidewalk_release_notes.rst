.. _sidewalk_release_notes:

Release notes
#############

See the release notes for the information about specific Sidewalk support releases for nRF Connect SDK.

[v1.14.5-dev1] - 12/07/2023
---------------------------

* Added experimental support for the nRF5340 development kit.
* Updated sdk-nrf to v2.4.99 (main).

[v1.14.4] - 06/05/2023
----------------------

* Released Sidewalk based on the nRF Connect SDK v2.4.0.
* Moved the Semtech pins configuration to DTS.
* Changed the DFU mode behavior.
  The device now resets before entering the DFU mode.
* Added notifying through LEDs whenever the device enters the DFU mode.
* Automated the repository maintenance with GitHub Actions.
* Added a link status for every transport in the serial log.
* Added the `AWS IoT Core tools repository`_ to the west manifest file.

[v1.14.3] - 03/22/2023
----------------------

* Released the nRF Connect SDK platform support for Sidewalk over Bluetooth LE, LoRa and FSK.

[v1.14.2] - 02/23/2023
----------------------

* Release candidate based on the nRF Connect SDK v2.3.0-rc1.

[v1.14.1] - 02/15/2023
----------------------

* Committed Sidewalk libraries v1.14.
* Added documentation preview for development.

.. _AWS IoT Core tools repository: https://github.com/aws-samples/aws-iot-core-for-amazon-sidewalk-sample-app

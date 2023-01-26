.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

24 Jan 2023
-----------
* KRKNWK-14583: Bus fault after flash, before manufacturer data flash.
    For LoRa sample, when there is no manufacturer hex, the device throws a hard fault after being powered up.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

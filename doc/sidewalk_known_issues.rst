.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

v1.14.4
-------
* KRKNWK-17035: Sensor monitor uplink messages are lost when the notification period is longer than 30 seconds.
    If the notification period is set to longer than 30 seconds, the sensor monitor uplink messages are lost. 
    For this reason, the notification period is set to 15 seconds by default.

* KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash.
    For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

* Dynamic FSK-WAN Profile switching from 2 to 1 is prohibited.
    Currently there is no workaround for this issue.

* Note for users with multiple Echo devices:
    If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
    If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

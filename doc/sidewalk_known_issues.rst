.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

v1.14.5-dev1
------------

* KRKNWK-17320: Bluetooth LE connection between Gateway and End Node drops within 30 seconds of being idle.
    The issue occurs only on the nRF5340 development kit (experimental support) and happens due to failing certification test.

* KRKNWK-17321: The device fails after multiple restarts.
   The issue occurs only on the nRF5340 development kit (experimental support) and happens due to failing certification test.

* KRKNWK-17308: Release version does not boot.
    The issue occurs only on the nRF5340 development kit (experimental support).

* KRKNWK-17114: Bootloader configuration is not optimized.
   The issue occurs only on the nRF5340 development kit (experimental support). 
   Due to large bootloader partition size (48 kB), the DFU always requires an external flash.

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

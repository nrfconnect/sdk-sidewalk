.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

v1.14.5-dev1
------------

* KRKNWK-17374: Sporadic Zephyr fatal error after disconnecting on FSK
    After disconnecting on FSK, Zephyr fatal error occurrs due to assertion in semaphore module.
    The error reproduces rarely - once per few days.
    In the release mode the device resets automatically, however, in the debug mode it need to be reset manually.
    Currently, this issue occurs for Sidewalk 1.14 libraries and it will be fixed in the version 1.16.

* KRKNWK-17321: The device fails after multiple restarts on the nRF5340 DK.
   The issue happens due to failing certification test.

   **Affected platforms:** nRF5340

* KRKNWK-17114: Bootloader configuration is not optimized on the nRF5340 DK.
   Due to large bootloader partition size (48 kB), the DFU always requires an external flash.

   **Affected platforms:** nRF5340

* KRKNWK-17035: Sensor monitor uplink messages are lost when the notification period is longer than 30 seconds
    If the notification period is set to longer than 30 seconds, the sensor monitor uplink messages are lost.
    For this reason, the notification period is set to 15 seconds by default.

* KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash
    For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

* Dynamic FSK-WAN Profile switching from 2 to 1 is prohibited
    The issue is a limitation for the 1.14 Sidewalk release.
    Currently there is no workaround for this issue.

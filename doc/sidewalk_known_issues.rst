.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

v1.14.5-dev1
------------

* KRKNWK-17860: QSPI assert occurs when performing DFU in the Sidewalk application mode
    The DFU should be performed only in the DFU mode.
    The DFU Bluetooth service can be used in the Sidewalk mode, however, using it leads to assertion failure, resulting in a Zephyr fatal error.

* KRKNWK-17800: After reconnecting to network, the end device cannot find the route to its host
    After the device disconnects from Sidewalk servers, the Sensor monitoring app over Bluetooth LE shows an error ``-38 (SID_ERROR_NO_ROUTE_AVAILABLE)``.
    To fix the error, the device needs to be reset manually.

* KRKNWK-17750: Error occurs when sending multiple messages in a short period of time
    When sending multiple Sidewalk messages in a short period of time, the internal queues might become full, showing misleading error messages, such as ``-12 (SID_ERROR_INCOMPATIBLE_PARAMS)``.
    The message should resend after some time needed for protocol to empty the queues.

* KRKNWK-17244:  CMake warnings when building the Sidewalk samples
    CMake warnings (``No SOURCES given to Zephyr library``) show in the build log of Sidewalk application. 
    Application builds successfully, but the error may obfuscate other important warnings.

* KRKNWK-17374: Sporadic Zephyr fatal error after disconnecting on FSK
    After disconnecting on FSK, Zephyr fatal error occurs due to assertion in semaphore module.
    The error reproduces rarely - once per few days.
    The device resets automatically in the release mode, however, in the debug mode it needs to be reset manually.
    Currently, this issue occurs for Sidewalk 1.14 libraries and it will be fixed in the version 1.16.

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

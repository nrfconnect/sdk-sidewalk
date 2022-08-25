.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

Alpha release
-------------
* KRKNWK-14235: When using No Optimization (-O0), the Zephyr assertion fails in Sidewalk init.
    SpinLock throws an assert only when ``CONFIG_NO_OPTIMIZATIONS=y`` is set.
    The temporary solution is to use ``CONFIG_DEBUG_OPTIMIZATION=y``, which provides a similar debugging experience and does not cause the issue.

* KRKNWK-14425: GPIO interrupt configuration issue.
    The GPIO callback handler is not invoked when the same pins are set on different ports, for example, P0.8 and P1.8.
    The issue occurs only when the GPIO ports are configured in the ascending order (from lowest to highest).
    This issue has already been fixed in upstream Zephyr in `PR 45537`_ and is waiting to be pulled into NCS.

* KRKNWK-14890: Fatal error on Button 2 in LoRa/FSK template sample due to assertion failure.
    In Bluetooth LE link mode you have to press **Button 2** (set connection request) to connect with Sidewalk.
    In LoRa/FSK, however, this is not required. Do not use **Button 2** when in LoRa/FSK mode.

* KRKNWK-14583: Bus fault after flash, before manufacturer data flash.
    For LoRa sample, when there is no manufacturer hex, the device throws a hard fault after being powered up.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

.. _PR 45537: https://github.com/zephyrproject-rtos/zephyr/pull/45537

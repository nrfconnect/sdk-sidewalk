.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

08/24/2022
----------

* KRKNWK-14235: With No Optimization (-O0) Zephyr assertion fails in Sidewalk init.

* KRKNWK-14890: Fatal error on button 2 in LoRa/FSK template sample due to assertion failure.
    In BLE link mode pressing the button 2 (set connection request) is required to connect with Sidewalk.
	In LoRa/FSK this is not required. Please don't use button 2 when in LoRa/FSK mode.

[Alpha release] - 07/21/2022
----------------------------

* KRKNWK-14583: Bus fault after flash, before manufacturer data flash.
    For LoRa sample, when there is no manufacturer hex, the device throws a hard fault after being powered up.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

* KRKNWK-14235: When using No Optimization, the Zephyr assertion fails in Sidewalk init.
    SpinLock throws an assert only when ``CONFIG_NO_OPTIMIZATIONS=y`` is set.
    The temporary solution is to use ``CONFIG_DEBUG_OPTIMIZATION=y``, which provides a similar debugging experience and does not cause the issue.

* KRKNWK-14425: GPIO interrupt configuration issue.
    The GPIO callback handler is not invoked when the same pins are set on different ports, for example, P0.8 and P1.8.
    The issue occurs only when the GPIO ports are configured in the ascending order (from lowest to highest).
    This issue has already been fixed in upstream Zephyr in `PR 45537`_ and is waiting to be pulled into NCS.

* KRKNWK-13807: Low power timers not supported.
    Current implementation does not support low power timers.
    This results in increased wake time of the MCU and therefore increased power usage.
    Power profiling and power optimization will be implemented in the future.

* KRKNWK-14684: LoRa sample freezes 30 seconds after a reset.
    The device is stuck in a loop 30 seconds after a reset.
    The issue is under the investigation, and currently it is suggested to avoid LoRa transport until the bug is fixed.
    Workaround: The issue manifests sporadically, it is possible that enabling log messages decrease probability of the bug.

.. _PR 45537: https://github.com/zephyrproject-rtos/zephyr/pull/45537

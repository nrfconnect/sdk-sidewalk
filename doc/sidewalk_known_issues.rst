.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

[Unreleased] - xxxx-xx-xx
-------------------------

* KRKNWK-14583: Bus fault after flash, before manufacturer data flash.
    For LoRa sample when there is no manufacturer hex, the device throws hard after power up.
    Proper error handling will be implemented, for now the temporary solution is to write manufacturing hex 
    to the device and reset.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr
    The non-resolvable private address (NRPA) cannot be set in connectable mode for BLE. 
    Currently there is no known workaround for this issue.

* KRKNWK-14235: With No Optimization Zephyr assertion fails in sidewalk init
    SpinLock throws an assert only when ``CONFIG_NO_OPTIMIZATIONS=y`` is set.
    temporary solution for this issue would be to use ``CONFIG_DEBUG_OPTIMIZATION=y``, 
    which provide similar debugging experienceand do not cause an issue.

* KRKNWK-14425: GPIO interrupt configuration issue.
    The GPIO callback handler is not invoked when the same pins are set on different ports, e.g. P0.8 and P1.8.
    The issue occur only when the GPIO ports are configured in order  from lower to higher.
    This issue has already been fixed in upstream Zephyr - 
    https://github.com/zephyrproject-rtos/zephyr/pull/45537, it is waiting to be pulled into NCS

* KRKNWK-13807: Implement low power timers.
    Current implementation do not support low power timers, which will increase wake time of the MCU and therefore power usage.
    Power profiling and power optimization will be added in the future.

* KRKNWK-14684: LoRa sample freezes after 30 seconds when reset has been performed.
    The device after reset gets stuck in endless loop after 30s after reset.
    The issue is under investigation and currently it is suggested to avoid LoRa transport until this bug is fixed.
    Workaround: Enabling debugs may significantly extend working time of LoRa sample. Investigations are pending.

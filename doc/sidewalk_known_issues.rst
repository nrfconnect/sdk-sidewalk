.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

4 Jan 2023
----------
* KRKNWK-14728: End Node sporadically freezes on LoRa.
    LoRa sample freezes after a reset when there is no successful time sync for 30 seconds since power up.
    The issue has reproduction rate below 20%.

* KRKNWK-14235: When using No Optimization (-O0), the Zephyr assertion fails in Sidewalk init.
    SpinLock throws an assert only when ``CONFIG_NO_OPTIMIZATIONS=y`` option is set.
    workaround: Use the default configuration. It is inconvenient, however, it makes the debugging possible.

* KRKNWK-14583: Bus fault after flash, before manufacturer data flash.
    For LoRa sample, when there is no manufacturer hex, the device throws a hard fault after being powered up.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

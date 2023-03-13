.. _sidewalk_known_issues:

Known issues
************

See the list of known issues that are valid for the current state of development.

02/01/2023
----------
* KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash.
    For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
    Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

* KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
    The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
    Currently there is no workaround for this issue.

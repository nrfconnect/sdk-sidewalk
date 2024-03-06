.. _sidewalk_release_notes:

Release notes
#############

For more information about the current Amazon Sidewalk release, refer to the links in the following sections.

Changelog
*********

See the `Release notes for the nRF Connect SDK`_ for the v2.6.0 release.

Limitations
***********

Dynamic FSK-WAN Profile switching from 2 to 1 is prohibited.
  The issue is a limitation of the 1.14 Sidewalk release.
  There is no workaround for this issue.

Known issues
************

.. note::
  Starting from the v2.5.0 release of the nRF Connect SDK, known issues have been moved to the nRF Connect SDK documentation.

See `Known issues for the nRF Connect SDK`_ for the v2.6.0 release.

Known issues (before the v2.5.0 release)
----------------------------------------

Up until the v2.5.0 release, Sidewalk was not a part of the nRF Connect SDK.
For this reason, numbering for previous releases is aligned with the official specification-based versioning.

v1.14.5-dev1
============

.. toggle::

    * KRKNWK-17320: Bluetooth LE connection between Gateway and End Node drops within 30 seconds of being idle
        The issue occurs only on the nRF5340 development kit (experimental support) and happens due to failing certification test.

    * KRKNWK-17321: The device fails after multiple restarts
        The issue occurs only on the nRF5340 development kit (experimental support) and happens due to failing certification test.

    * KRKNWK-17308: Release version does not boot
        The issue occurs only on the nRF5340 development kit (experimental support).

    * KRKNWK-17114: Bootloader configuration is not optimized
        The issue occurs only on the nRF5340 development kit (experimental support).
        Due to large bootloader partition size (48 kB), the DFU always requires an external flash.

    * KRKNWK-17035: Sensor monitor uplink messages are lost when the notification period is longer than 30 seconds
        If the notification period is set to longer than 30 seconds, the sensor monitor uplink messages are lost.

        **Workaround:** The notification period is set to 15 seconds by default.

    * KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash
        For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
        Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

    * KRKNWK-14299: NRPA MAC address cannot be set in Zephyr
        The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
        Currently there is no workaround for this issue.

v1.14.4
=======

.. toggle::

    * KRKNWK-17035: Sensor monitor uplink messages are lost when the notification period is longer than 30 seconds
        If the notification period is set to longer than 30 seconds, the sensor monitor uplink messages are lost.

        **Workaround:** The notification period is set to 15 seconds by default.

    * KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash
        For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
        Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

    * KRKNWK-14299: NRPA MAC address cannot be set in Zephyr
        The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
        Currently there is no workaround for this issue.


v1.14.3
=======

.. toggle::

    * KRKNWK-16402: DFU failure on unregistered devices.
        DFU fails on a device that is not registered.

        **Workaround:** Enter the DFU mode when the device is registered.

    * KRKNWK-14583: Bus fault after flash, before the :file:`Nordic_MFG.hex` data flash.
        For sub-GHz samples, when the :file:`Nordic_MFG.hex` file is missing, the device throws a hard fault during initializing the Sidewalk stack.
        Proper error handling will be implemented, but the temporary solution is to write manufacturing hex to the device and reset it.

    * KRKNWK-14299: NRPA MAC address cannot be set in Zephyr.
        The non-resolvable private address (NRPA) cannot be set in the connectable mode for Bluetooth LE.
        Currently there is no workaround for this issue.

.. include:: ncs_links.rst

.. _sidewalk_release_notes_addon_v200:

Release notes for Amazon Sidewalk Add-On v2.0.0
###############################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates in Amazon Sidewalk Add-On v2.0.0-rc1 as compared to v1.3.0.
This release introduces the Amazon Sidewalk SDK v2.0 libraries.
For more information refer to the following sections.

For the list of potential issues, see the `known issues page on the main branch`_.

Amazon Sidewalk SDK v2.0 highlights
***********************************

The Amazon Sidewalk SDK v2.0 is a major release of the Sidewalk end device stack.
The following list briefly describes its new features and their support in this add-on release.
For the complete list of changes, see the `Amazon Sidewalk MCU SDK Migration Guide`_.

* Europe region - Besides North America, the Sidewalk network is available in a subset of European countries.
  For LoRa and FSK, end devices operate in the Europe region (868 MHz band), with lower transmit power and airtime limits.
  The add-on supports both regions, selected at build time with the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice.
  See :ref:`sidewalk_configuration` and the `Amazon Sidewalk Internationalization Application Note`_.

* Gateway Session Key (GSK) - The end device and the gateways in its area share a session key on the FSK and LoRa links, so the gateway can verify the uplinks locally, without the cloud.
  GSK is enabled by default in the Sidewalk libraries and is used when the Amazon Sidewalk network enables it for the link.
  When a GSK session is active on LoRa, the link runs at the medium data rate (MDR).
  See the `Amazon Sidewalk Gateway Session Key (GSK) Application Note`_.

* LoRa medium data rate (MDR) - A faster LoRa data rate with link-layer acknowledgements, available in North America only.
  In Europe, LoRa always uses the low data rate (LDR).

* Location services - The Location Library adds the FSK gateway location (L2) and the LoRa multilateration (L5), also called trilateration.
  LoRa multilateration resolves the device position from a LoRa uplink received by several gateways, without additional location hardware.
  The application can also provide its own ordered list of location levels.
  See :ref:`location_services`.

* Registration over Wire (ROW) - Devices can be provisioned and registered on the factory line, over UART or RTT, with the ``sid-mfg-tool`` host tool.
  The prebuilt libraries include the ROW stack, but the add-on does not provide a Factory Diagnostic Firmware with ROW support yet.
  See the `Amazon Sidewalk Registration over Wire Application Note`_.

* Keep-alive payload and acknowledgement event - On FSK and LoRa (Profile B and C), the application can attach a custom payload to the keep-alive frames and is notified whether each keep-alive was acknowledged.

* FSK beacon events - The application is notified about the received and missed FSK beacons.
  See the `Amazon Sidewalk Gateway Discovery and Selection Application Note`_.

Changelog
*********

* Added:

  * Support for the Europe region with the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice.
    Use ``CONFIG_SIDEWALK_SUBGHZ_REGION_US915`` (default) for North America or ``CONFIG_SIDEWALK_SUBGHZ_REGION_EU868`` for Europe.
  * Regulatory region configuration in the Sidewalk platform initialization, required by the Sidewalk v2.0 libraries to create the LoRa and FSK MACs.
  * The ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` (default) and ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig options for selecting the release or debug variant of the prebuilt Sidewalk libraries.
    See :ref:`sidewalk_configuration`.
  * Location shell commands in the :ref:`variant_sidewalk_dut` variant:

    * ``location send_levels`` - Tries the given location levels in order.
    * ``location send_pseudorange`` - Sends a raw pseudorange payload, given as a hex string, as is.
    * ``location manage_effort`` - Shows or sets the managed effort mode of the Location Library.

  * Extra shell commands in the :ref:`variant_sidewalk_dut` variant: top-level ``print`` and ``metrics`` commands, and optional beacon interval and RX window duration parameters of ``sid option -lp_set 2``.
  * Function for reading the current Bluetooth LE advertising address in the Bluetooth LE platform abstraction layer.

* Updated:

  * The nRF Connect SDK from v3.4.0 to v3.4.1.
  * The Amazon Sidewalk libraries from v1.19.4 to the v2.0.0.60 release.
  * The sub-GHz radio platform abstraction layer to the Sidewalk v2.0 interface.
  * The :ref:`sidewalk_end_device` sample to the Sidewalk v2.0 Location Library API (typed location results).
  * The ``SIDEWALK_MFG_STORAGE_DIAGNOSTIC`` Kconfig option renamed to ``SIDEWALK_MFG_ALLOW_WRITE``.
  * The :ref:`sidewalk_end_device` sample configuration enables ``CONFIG_PM_DEVICE_RUNTIME``, so the radio SPI bus is suspended between transfers, which lowers the sleep current with the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`.
  * The :ref:`location_services` documentation with the FSK gateway location and the LoRa multilateration.
  * The note about the regions where Amazon Sidewalk is available, and the information about testing outside of the supported countries.

* Fixed:

  * The ``cert store`` command of the on-device certification failing with ``SID_ERROR_NOSUPPORT`` on the nRF54L Series.
  * Parsing of the certificate authority serial number in the on-device certification.
  * An issue where Bluetooth LE advertising started during an active Bluetooth LE connection and leaked the connection reference.
  * An issue where the Bluetooth LE connection mutex handling did not work correctly.
  * The Bluetooth LE link not being disconnected on the Sidewalk deinitialization.
  * The Semtech LR11xx interrupt line remaining in the high state during a Wi-Fi scan.
  * Slow ``location init`` caused by the log flush waiting for the periodic wakeup of the log thread.

Backward compatibility
**********************

The Amazon Sidewalk SDK v2.0 changes parts of the application-facing Sidewalk API, mainly the Location Library.
For migration details, see the :ref:`migration_guide_addon_v200` page.

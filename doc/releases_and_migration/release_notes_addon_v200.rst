.. _sidewalk_release_notes_addon_v200:

Release notes for Amazon Sidewalk Add-On v2.0.0
###############################################

.. contents::
   :local:
   :depth: 2

This page describes Amazon Sidewalk Add-On v2.0.0 compared with v1.3.0, including the Sidewalk SDK v2.0 libraries.

For known issues, see the `known issues page on the main branch`_.

.. _sidewalk_release_notes_v200_highlights:

Amazon Sidewalk SDK v2.0 highlights
***********************************

The Sidewalk SDK v2.0 is a major release of the Sidewalk end device stack.
For the complete SDK change list, and how to migrate to the newer Sidewalk SDK version, see the `Amazon Sidewalk MCU SDK Migration Guide`_.

* Europe region — In addition to North America, Sidewalk is available in a subset of European countries.
  For LoRa and FSK, end devices use the European 868 MHz band, with lower transmit power and airtime limits than in North America.
  This add-on supports both regions through the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice at build time.
  See :ref:`sidewalk_configuration` and the `Amazon Sidewalk Internationalization Application Note`_.

* Location services — Extends the Location Library with FSK gateway location (L2) and LoRa multilateration (L5), also called trilateration.
  LoRa multilateration derives position from a LoRa uplink received by several gateways, without extra location hardware.
  Applications can also supply an ordered list of location levels to try.
  See :ref:`location_services`.

* Registration over Wire (ROW) — The Sidewalk libraries support factory provisioning and in-factory registration over UART or RTT from a host PC using Amazon's ``sid-mfg-tool``.
  In this add-on, ROW support is experimental and integrated in the Device Under Test (DUT) sample for evaluation.
  See the `Amazon Sidewalk Registration over Wire Application Note`_.

* Gateway Session Key (GSK) — The end device and nearby gateways share a session key on FSK and LoRa links so gateways can verify uplinks locally without sending traffic to the cloud.
  GSK is enabled by default in the Sidewalk libraries and applies when the Amazon Sidewalk network enables it for the link.
  When a GSK session is active on LoRa, the link uses the medium data rate (MDR).
  See the `Amazon Sidewalk Gateway Session Key (GSK) Application Note`_.

* LoRa medium data rate (MDR) — Adds a faster LoRa data rate with link-layer acknowledgements in North America only.
  In Europe, LoRa always uses the low data rate (LDR).

* Keep-alive payload and acknowledgement event — On FSK and LoRa (Profile B and C), applications can attach a custom payload to keep-alive frames and receive an acknowledgement status for each keep-alive.

* FSK beacon events — Applications receive callbacks for received and missed FSK beacons.
  See the `Amazon Sidewalk Gateway Discovery and Selection Application Note`_.

Changelog
*********

The following list covers add-on-specific changes.

* Added:

  * Support for the Europe region with the ``SIDEWALK_SUBGHZ_REGION`` Kconfig choice.
    Use ``CONFIG_SIDEWALK_SUBGHZ_REGION_US915`` (default) for North America or ``CONFIG_SIDEWALK_SUBGHZ_REGION_EU868`` for Europe.
  * Regulatory region configuration in Sidewalk platform initialization, required by the v2.0 libraries to create the LoRa and FSK MACs.
  * The ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` (default) and ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig options for selecting the release or debug variant of the prebuilt Sidewalk libraries.
    See :ref:`sidewalk_configuration`.
  * Location shell commands in the :ref:`variant_sidewalk_dut` variant:

    * ``location send_levels`` — Tries the given location levels in order.
    * ``location send_pseudorange`` — Sends a raw pseudorange payload as a hex string, unchanged.
    * ``location manage_effort`` — Shows or sets the managed effort mode of the Location Library.

  * Additional DUT shell commands (``print``, ``metrics``, and extended ``sid option -lp_set 2`` parameters for beacon interval and RX window duration).
  * A function for reading the current Bluetooth LE advertising address in the Bluetooth LE platform abstraction layer.

* Updated:

  * The nRF Connect SDK from v3.4.0 to v3.4.1.
  * The Amazon Sidewalk libraries from v1.19.4 to v2.0.0.60.
  * The sub-GHz radio platform abstraction layer to the Sidewalk v2.0 interface.
  * The :ref:`sidewalk_end_device` sample:

    * Migrated to the Sidewalk v2.0 Location Library API (typed location results).
    * Enabled the ``CONFIG_PM_DEVICE_RUNTIME`` Kconfig option so the radio SPI bus suspends between transfers, lowering sleep current with the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`.
    * In the :ref:`variant_sidewalk_dut` variant, shell commands are provided by the Sidewalk QA library command-line interface in the Sidewalk libraries, following the Amazon Sidewalk reference CLI used for qualification testing, instead of a sample-specific Zephyr shell implementation.

  * Renamed the ``SIDEWALK_MFG_STORAGE_DIAGNOSTIC`` Kconfig option to ``SIDEWALK_MFG_ALLOW_WRITE``.
  * Documentation on supported Sidewalk regions and on testing outside those regions.
  * The :ref:`location_services` page with FSK gateway location and LoRa multilateration.

* Fixed:

  * On-device certification ``cert store`` command failing with ``SID_ERROR_NOSUPPORT`` on the nRF54L Series. (KRKNWK-22489)
  * On-device certification parsing of the certificate authority serial number.
  * Bluetooth LE advertising starting during an active connection and leaking the connection reference.
  * Bluetooth LE connection mutex handling during concurrent connection and advertising use.
  * Bluetooth LE link remaining connected after Sidewalk deinitialization.
  * Semtech LR11xx interrupt line remaining high during a Wi-Fi scan. (KRKNWK-21160)
  * LR1110 interrupt line falsely triggered by level shifters during radio sleep on the :ref:`nRF Sidewalk EB <nrf_sidewalk_eb>`, causing radio timeouts and endless loops.
  * Slow ``location init`` when log flush waited for the periodic log thread wakeup.

Backward compatibility
**********************

The Sidewalk SDK v2.0 breaks backward compatibility for the Location Library and related messaging APIs.
Plan a migration if you use the Location Library or custom ``sid_config`` or sub-GHz initialization.
See the :ref:`migration_guide_addon_v200` page.

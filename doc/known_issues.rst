.. _known_issues:

Known issues
############

.. contents::
   :local:
   :depth: 2

Known issues listed on this page are valid for the current state of development.
A known issue can list one or both of the following entries:

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

List of known issues for v1.1.0
*******************************

KRKNWK-20869: Intermittent downlink message reception issues with multilink configurations
  When multiple transport types are enabled (multilink), downlink messages from the cloud may not be received by the device.
  Sometimes, messages arrive through a different transport than expected.
  Uplink messages work as expected.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield (the exact set of affected platforms is not constant, and reproduction is not consistent)

KRKNWK-20851: Cannot initialize location when LoRa or FSK transport is initialized in Amazon Sidewalk
  Device fails to initialize location when using LoRa or FSK transport.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield (issue reproduction is inconsistent)

  **Workaround:** If the device freezes when location initialization fails, restart the device.
    Ensure that location services are initialized only after the Sidewalk stack is ready, and deinitialized before Sidewalk stack is deinitialized.

KRKNWK-20863: Increased boot time due to bootloader configuration
  Boot time has increased on all platforms due to bootloader configuration changes.
  It now uses RSA keys for signature verification ((the default config of ``BOOT_SIGNATURE_TYPE`` is changed to ``BOOT_SIGNATURE_TYPE_RSA``), significantly increasing the boot time (commit 99ed5147cb7543c1f8f9a116da801d3909a92e83, "use rsa keys for all SoCs").
  The effect is most noticeable on the nRF52840 SoC (around 3 seconds to start up instead of the expected less than 1 second).
  This issue is unrelated to specific radio configurations (LoRa, Bluetooth LE) but affects the general boot process.

  **Affected platforms:** All platforms

KRKNWK-20856: GNSS scan enters endless loop
  The GNSS scan operation may enter an endless loop.
  Reproduction of the issue is inconsistent.
  It does not occur with the location send command.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield

  **Workaround:** Reset device when freezes in endless GNSS loop.

KRKNWK-20857: Location modem stays busy when scan operation overlaps with deinit
  The location modem does not recover if a scan runs concurrently with a deinitialization operation.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield

  **Workaround:** If the location modem stays busy, and cannot perform scan restart the device.
    Ensure the location service deinit is triggered when no ongoing scan.

KRKNWK-21159: Crash in ``sid_location_run`` due to invalid context in Wi-Fi callback
  The application may crash if ``sid_location_run`` is called in an invalid context during a Wi-Fi callback.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield

KRKNWK-21160: Semtech interrupt pin remains high blocking the Wi-Fi scan.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield

  **Workaround:** Clear event pin interrupt in Semtech hal after each occurrence.

KRKNWK-21514: Sidewalk option for Bluetooth LE config (``SID_OPTION_BLE_USER_CONFIG``) is not supported in the NCS
  Calling the Sidewalk Kconfig option ``SID_OPTION_BLE_USER_CONFIG`` causes the application to crash.

  **Affected platforms:** All platforms.

List of known issues for v1.0.1
*******************************

KRKNWK-19950: SBDT speed degrades when using the v1.18 libraries
  There is a noticeable degradation in the speed of the SBDT when using the version 1.18 of Amazon Sidewalk libraries.

KRKNWK-19948: There is a mismatch in ACK parameters between the ``send`` function and ``on_msg_sent`` JSON output
  The ACK parameters utilized by the ``send`` function do not show correctly in the callback and are incorrectly set in the UART debug logs.

KRKNWK-19119: Multi-image DFU fails to restore after a sample reset.
  The multi-image DFU does not successfully restore following a sample reset.

  **Workaround:** If the device is reset during the DFU process, the DFU always restarts from the beginning.

KRKNWK-20330: The Amazon Sidewalk application crashes on startup when the ``CONFIG_NANOPB`` Kconfig option is enabled
  The Amazon Sidewalk libraries include a nanopb implementation that conflicts with the one in Zephyr, causing the application to crash immediately after startup.

  **Workaround:** Disable the ``CONFIG_NANOPB`` Kconfig option and enable the ``CONFIG_SIDEWALK_NANOPB`` Kconfig option instead.
  This configuration uses the nanopb implementation from the Amazon Sidewalk libraries, allowing the application to start successfully.

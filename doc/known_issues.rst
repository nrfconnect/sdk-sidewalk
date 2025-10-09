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

.. warning::
   SDK v1.19 introduces several new issues. Thorough testing is recommended before production use.

**High Priority Issues:**

KRKNWK-20869: Intermittent downlink message reception issues with multilink configurations
  When multiple transport types are enabled (multilink), downlink messages from the cloud may not be received by the device.
  Sometimes messages arrive via a different transport than expected. Uplink messages work as expected.

  
  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield
  
  **Note:** The exact set of affected platforms that show this behavior is not constant, and reproduction of this issue is not consistent.
  
  **Impact:** Moderate - affects message reliability in multilink scenarios

**Lower Priority Issues:**

KRKNWK-20863: Increased boot time due to bootloader configuration
  Boot time increased on all platforms due to recent bootloader configuration changes.
  The impact is most noticeable on nRF52840 (~3 seconds vs expected <1 second), but all platforms experience
  increased boot time.
  
  **Root Cause:** The increased boot time is caused by bootloader configuration changes that affect the
  initialization sequence across all platforms.
  
  **Technical Details:** 
  - **Commit:** `99ed5147cb7543c1f8f9a116da801d3909a92e83` - "use rsa keys for all SoCs"
  - **Change:** Sets default config of `BOOT_SIGNATURE_TYPE` to `BOOT_SIGNATURE_TYPE_RSA`
  - **Impact:** RSA signature verification is computationally more expensive than the previous default,
    causing increased boot time across all platforms
  
  **Note:** This issue is not related to specific radio configurations (LoRa, BLE, etc.) but affects
  the general boot process on all platforms.
  
  **Affected platforms:** All platforms (most significant impact on nRF52840)

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

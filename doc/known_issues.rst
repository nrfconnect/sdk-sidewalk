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

List of known issues for v1.3.0
*******************************

KRKNWK-20891: Multilink downlinks are sent over an incorrect transport

  When multilink is configured (for example, Bluetooth LE and LoRa), downlink messages sent from the cloud are always received over LoRa, regardless of the transport used for the corresponding uplink.
  The expected behavior is that the downlink uses the same transport as the uplink.

  **Affected platforms:** The nRF52840 SoC with the LR1110 shield.

KRKNWK-22012: ``test_power_profile`` fails on the LR1110 shield

  On FSK builds, switching power profiles (``sid option -lp_set 0x02 630``) might return ``SID_ERROR_INVALID_ARGS`` (-11), followed by a PSA crypto error (-149) in ``sid_pal_crypto_aead_crypt``.
  In such cases, the device does not become ready again within the expected time.
  On LoRa builds, switching profiles might leave the device unable to recover the ready state, or leave the link down.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield.

KRKNWK-22011: ``sid send`` fails during the Bluetooth LE multilink certification test (M02)

  The multilink certification test sends the uplink over Bluetooth LE while LoRa is the active link mode.
  In this scenario, a PSA AEAD crypto error (-149) might occur before the Bluetooth LE send.
  The error is followed by a Bluetooth LE disconnect, and ``sid send`` returns error -40.
  The device does not receive the ``on_msg_sent`` callback.

  **Affected platforms:** The nRF54L15 SoC with the LR1110 shield.

KRKNWK-20371: SBDT multi-image DFU does not resume after a reset

  If the device resets while a multi-image DFU (SBDT) transfer is in progress, the transfer does not resume and starts from the beginning.

  **Workaround:** Do not reset the device during a multi-image DFU transfer.
  If a reset occurs, restart the DFU transfer.

KRKNWK-16256: EN does not drop the Bluetooth LE connection after receiving a duplicated message with the same sequence number

  During certification testing, the cloud sends a downlink message with a sequence number that was already received.
  The end node does not transition to the ``SID_STATE_NOT_READY`` state, and does not drop the Bluetooth LE connection.
  This causes the ``test_CERT_CB04`` and ``test_CERT_CB05`` certification tests to fail.

  **Affected platforms:** The nRF52840 SoC.

KRKNWK-20854: ``sid_location_run`` ignores the location initialization configuration

  When location is initialized with Bluetooth LE support only (for example, for the SX1262 shield), calling ``location send 3`` or ``location send 4`` (GNSS or Wi-Fi scan) does not return an error, even though GNSS and Wi-Fi are not initialized.
  The mocked GNSS and Wi-Fi functions return a not-supported error internally, but the error does not reach the user.

  **Affected platforms:** All platforms using the SX1262 shield.

KRKNWK-20857: Location modem stays busy when scan operation overlaps with deinit

  The location modem does not recover if a scan runs concurrently with a deinitialization operation.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield.

  **Workaround:** If the location modem stays busy, and cannot perform the scan, restart the device.
    Ensure the location service deinit is triggered when there is no ongoing scan.

KRKNWK-22410: Registration and deregistration over FSK is not verified against the certification tests

  Registration and deregistration of the device over the FSK transport is not verified against the ``FSK/EP/CONN/REG/BV/02`` and ``FSK/EP/CONN/REG/BV/03`` certification test signatures.
  An earlier occurrence of this failure mode, where the end node never registered over FSK, was caused by a chip select timing delay in the Zephyr SPI driver that made the device miss the FSK receive windows.
  It was fixed by using the nrfx SPI driver directly, but the fix is not confirmed against the current test signatures.

  **Affected platforms:** All platforms using the LR1110 or SX1262 shield.

KRKNWK-22411: Uplink and downlink exchange during UUID rotation is not verified for FSK and LoRa

  Exchanging uplink and downlink messages while a UUID rotation is in progress is not verified against the ``FSK/EP/SEC/UUID/BV/03`` and ``LORA/EP/SEC/UUID/BV/03`` certification test signatures.

  **Affected platforms:** All platforms using the LR1110 or SX1262 shield.

List of known issues for v1.2.0
*******************************

KRKNWK-20869: Intermittent downlink message reception issues with multilink configurations

  When multiple transport types are enabled (multilink), the device might not receive downlink messages from the cloud.
  Sometimes, messages arrive through a different transport than expected.
  Uplink messages work as expected.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield.
  However, the exact range of affected platforms is not constant, and reproduction is not consistent.

KRKNWK-20851: Cannot initialize location when LoRa or FSK transport is initialized in Amazon Sidewalk

  Device fails to initialize location when using LoRa or FSK transport.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield (issue reproduction is inconsistent)


  **Workaround:** Ensure that location services are initialized only after the Sidewalk stack is ready, and deinitialized before Sidewalk stack is deinitialized.

KRKNWK-20863: Increased boot time due to bootloader configuration

  Boot time has increased on all platforms due to bootloader configuration changes.
  It now uses RSA keys for signature verification (the default of ``BOOT_SIGNATURE_TYPE`` Kconfig option is changed to ``BOOT_SIGNATURE_TYPE_RSA``), significantly increasing the boot time (commit ``99ed5147cb7543c1f8f9a116da801d3909a92e83`` "use rsa keys for all SoCs").
  The effect is most noticeable on the nRF52840 SoC (around 3 seconds to start up instead of the expected less than 1 second).
  This issue is unrelated to specific radio configurations (LoRa, Bluetooth LE) but affects the general boot process.

  **Affected platforms:** All platforms

KRKNWK-20856: GNSS scan enters endless loop

  The GNSS scan operation may enter an endless loop.
  Reproduction of the issue is inconsistent.
  It does not occur with the location send command.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

  **Workaround:** Reset device when it freezes in endless GNSS loop.

KRKNWK-20857: Location modem stays busy when scan operation overlaps with deinit

  The location modem does not recover if a scan runs concurrently with a deinitialization operation.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

  **Workaround:** If the location modem stays busy, and cannot perform the scan, restart the device.
    Ensure the location service deinit is triggered when there is no ongoing scan.

KRKNWK-21160: Semtech interrupt line is stuck high, preventing the Wi-Fi scan from running

  The Semtech radio interrupt pin may remain high during a Wi-Fi location scan, which blocks the scan from completing.
  This can occur when location scanning overlaps with communication on the Semtech radio.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

  **Workaround:** Clear event pin interrupt in Semtech hal after each occurrence.

KRKNWK-21159: Crash in ``sid_location_run`` due to invalid context in Wi-Fi callback

  The application may crash if ``sid_location_run`` is called in an invalid context during a Wi-Fi callback.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

KRKNWK-22208: Bluetooth LE advertising fails after Sidewalk deinit and reinit cycle

  After a full Sidewalk Bluetooth LE deinitialization and reinitialization cycle, advertising may fail to restart.
  When a Bluetooth LE connection is established, the Bluetooth stack stops advertising, but the internal advertising state may remain stale.
  On the next ``sid_start()`` call, ``sid_ble_advert_update()`` might call the Bluetooth HCI API with an invalid advertising set.
  This results in an ``Advertising failed to update (err -22)`` error and causes ``sid_start()`` to return ``SID_ERROR_GENERIC``.

  **Affected platforms:** All platforms.

  **Workaround:** Reset the device after ``sid deinit`` and before calling ``sid init`` again. 
  Alternatively, avoid the deinit and reinit cycle unless you perform a full device reset.

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
  It now uses RSA keys for signature verification (the default config of ``BOOT_SIGNATURE_TYPE`` is changed to ``BOOT_SIGNATURE_TYPE_RSA``), significantly increasing the boot time (commit 99ed5147cb7543c1f8f9a116da801d3909a92e83, "use rsa keys for all SoCs").
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

KRKNWK-21160: Semtech interrupt pin remains high blocking the Wi-Fi scan

  The Semtech radio interrupt pin may remain high during a Wi-Fi location scan, which blocks the scan from completing.
  This can occur when location scanning overlaps with communication on the Semtech radio.

  **Affected platforms:** nRF52840, nRF54L15 with LR1110 shield

  **Workaround:** Clear event pin interrupt in Semtech hal after each occurrence.

KRKNWK-21514: Sidewalk option for Bluetooth LE config (``SID_OPTION_BLE_USER_CONFIG``) is not supported in the NCS

  Calling the Sidewalk Kconfig option ``SID_OPTION_BLE_USER_CONFIG`` causes the application to crash.

  **Affected platforms:** All platforms.

  Fixed in Amazon Sidewalk Add-on v1.2.0.

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

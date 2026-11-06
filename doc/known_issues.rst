:orphan:

.. _known_issues:

Known issues
############

.. known-issues-filter::
   :default: 2.0.0

Known issues listed on this page are tagged with the add-on release they apply to.
Use the release filter above to view issues for a specific release.
A known issue can list one or more of the following entries:

* **Failing Test Cases:**

  For details about the listed test cases, see the `Test Specification`_.

* **Affected platforms:**

  If a known issue does not have any specific platforms listed, it is valid for all hardware platforms.

* **Workaround:**

  Some known issues have a workaround.
  Sometimes, they are discovered later and added over time.

.. contents::
   :local:
   :depth: 2

..
   This page stays ``:orphan:`` so it is not linked from the doc set navigation until release.
   Release notes use the ``known issues page on the main branch`` link in :file:`links.rst` instead of ``:ref:`known_issues``` so versioned doc bundles always point at the latest known issues list.

..
   When adding a new release, add its version to :file:`doc/versions.json` and set it as the default on the filter directive above.
   Tag each issue with ``.. rst-class:: vX-X-X`` (use a hyphen instead of a dot in the version).
   To mark an issue as won't fix, add the ``wontfix`` class.

.. rst-class:: v2-0-0

KRKNWK-22725: ``sid_location_init()`` rejects ``max_effort`` values above L4
  The Sidewalk SDK caps ``max_effort`` at L4 (or lower depending on enabled hardware).
  If you set ``sid_location_type_mask`` to ``SID_LOCATION_METHOD_ALL`` and ``max_effort`` to ``SID_LOCATION_EFFORT_L5``, ``sid_location_init()`` returns ``SID_ERROR_INVALID_ARGS``.
  As a result, managed effort cannot escalate to L5 (LoRa multilateration).

  **Affected platforms:** All platforms with location support.

  **Workaround:** When the LoRa link is available, request L5 explicitly, for example, using ``location send 5``.
  The ``max_effort`` setting only limits automatic escalation when no effort level is specified.
  It does not block explicit L5 requests.

.. rst-class:: v2-0-0

KRKNWK-22714: ``sid_location_init()`` succeeds when location is already initialized
  If you call ``sid_location_init()`` a second time, it returns ``SID_ERROR_NONE``, even if the first call failed or the location library is already initialized.
  However, the second call does not apply new settings, such as ``manage_effort``, so the library continues to use the previous configuration.

  **Affected platforms:** All platforms with location support.

  **Workaround:** To apply new settings, call ``location deinit`` before you initialize location again.
  If initialization fails, deinitialize location or reset the device before you try again.

.. rst-class:: v2-0-0

KRKNWK-22709: Calling ``sid_stop()`` during a location scan leaves the location modem busy
  If you call ``sid_stop()`` while a Wi-Fi or GNSS location scan is in progress, the location modem can remain busy until the device is reset.
  While the modem is busy, new scan requests return success but do not run, and ``location deinit`` and ``sid deinit`` return ``SID_ERROR_BUSY``.

  **Affected platforms:** All platforms with LoRa or FSK support and Wi-Fi or GNSS location (for example, LR1110-based shields).

  **Workaround:** Do not call ``sid_stop()`` or ``sid_deinit()`` when there is an ongoing location scan.
  Either wait for the scan to finish, or call ``location deinit`` first.
  In this release, ``location deinit`` waits for the scan to complete before returning.
  If the modem remains busy, reset the device.

.. rst-class:: v1-3-0 v2-0-0

KRKNWK-22693: LR11xx idle current remains high after a Wi-Fi or GNSS location scan
  When a Wi-Fi or GNSS location scan finishes, the LoRa Basics Modem planner puts the LR11xx into sleep mode.
  However, ``sid_pal_radio_release_scan()`` then restores the Sidewalk radio configuration, which wakes the LR11xx.
  The LR11xx then stays in standby mode until the next radio operation, which increases idle current by about 1 mA.

  **Affected platforms:** nRF52840 and nRF54L Series devices with an LR11xx radio, such as LR1110-based shields.

  **Workaround:** Apply the fix from `sdk-sidewalk pull request 850`_, or put the LR11xx back into sleep mode after each Wi-Fi or GNSS scan.

.. rst-class:: v1-3-0

KRKNWK-22489: On-device certification cannot store the manufacturing page
  The ``cert store`` command of the on-device certification CLI (``CONFIG_SIDEWALK_ON_DEV_CERT``) returns ``SID_ERROR_NOSUPPORT`` (-6) and the generated credentials are not stored.
  The manufacturing storage writes each TLV entry with a 4-byte alignment, while the RRAM write block size is 16 bytes, so the flash driver rejects the write.

  **Failing Test Cases:**

  - None.

  **Affected platforms:** All nRF54L Series platforms.

.. rst-class:: v1-3-0

KRKNWK-22012: Switching power profiles fails on FSK builds when using shell commands
  On FSK builds, switching power profiles (``sid option -lp_set 0x02 630``) might return ``SID_ERROR_INVALID_ARGS`` (-11).
  In such cases, the device does not become ready again within the expected time.

  **Failing Test Cases:**

  - ``FSK/EP/PWR/OPT/BV/01``: Verify RX duration can be adjusted.

  **Affected platforms:** All platforms with FSK support.

  **Workaround:** Call the equivalent ``sid_option()`` API directly.
  The issue is caused by command parser in the DUT sample application CLI, not the Sidewalk stack.

.. rst-class:: v1-3-0 v2-0-0

KRKNWK-20371: SBDT multi-image DFU does not resume after a reset
  If the device resets while a multi-image DFU (SBDT) transfer is in progress, the transfer does not resume and starts from the beginning.

  **Failing Test Cases:**

  - ``BLE/EP/API/FILETRANSFER/RECOVERY/BI/01``: Recover transfer after EP powered off and back on.

  **Affected platforms:** All platforms.

  **Workaround:** Do not reset the device during a multi-image DFU transfer.
  If a reset occurs, restart the DFU transfer.

.. rst-class:: v1-1-0 v1-2-0 v1-3-0

KRKNWK-20857: Location modem stays busy when scan operation overlaps with deinit
  The location modem does not recover if a scan runs concurrently with a deinitialization operation.

  **Failing Test Cases:**

  - None.

  **Affected platforms:** All platforms with LoRa or FSK support.

  **Workaround:** If the location modem stays busy, and cannot perform the scan, restart the device.
  Ensure the location service deinit is triggered when there is no ongoing scan.

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-20869: Intermittent downlink message reception issues with multilink configurations
  When multiple transport types are enabled (multilink), the device might not receive downlink messages from the cloud.
  Sometimes, messages arrive through a different transport than expected.
  Uplink messages work as expected.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield.
  However, the exact range of affected platforms is not constant, and reproduction is not consistent.

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-20851: Cannot initialize location when LoRa or FSK transport is initialized in Amazon Sidewalk
  Device fails to initialize location when using LoRa or FSK transport.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield (issue reproduction is inconsistent)

  **Workaround:** Ensure that location services are initialized only after the Sidewalk stack is ready, and deinitialized before Sidewalk stack is deinitialized.

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-20863: Increased boot time due to bootloader configuration
  Boot time has increased on all platforms due to bootloader configuration changes.
  It now uses RSA keys for signature verification (the default of ``BOOT_SIGNATURE_TYPE`` Kconfig option is changed to ``BOOT_SIGNATURE_TYPE_RSA``), significantly increasing the boot time (commit ``99ed5147cb7543c1f8f9a116da801d3909a92e83`` "use rsa keys for all SoCs").
  The effect is most noticeable on the nRF52840 SoC (around 3 seconds to start up instead of the expected less than 1 second).
  This issue is unrelated to specific radio configurations (LoRa, Bluetooth LE) but affects the general boot process.

  **Affected platforms:** All platforms

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-20856: GNSS scan enters endless loop
  The GNSS scan operation may enter an endless loop.
  Reproduction of the issue is inconsistent.
  It does not occur with the location send command.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

  **Workaround:** Reset device when it freezes in endless GNSS loop.

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-21160: Semtech interrupt line is stuck high, preventing the Wi-Fi scan from running
  The Semtech radio interrupt pin may remain high during a Wi-Fi location scan, which blocks the scan from completing.
  This can occur when location scanning overlaps with communication on the Semtech radio.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

  **Workaround:** Clear event pin interrupt in Semtech hal after each occurrence.

.. rst-class:: v1-1-0 v1-2-0

KRKNWK-21159: Crash in ``sid_location_run`` due to invalid context in Wi-Fi callback
  The application may crash if ``sid_location_run`` is called in an invalid context during a Wi-Fi callback.

  **Affected platforms:** The nRF52840 SoC and the nRF54L15 SoC with the LR1110 shield

.. rst-class:: v1-2-0

KRKNWK-22208: Bluetooth LE advertising fails after Sidewalk deinit and reinit cycle
  After a full Sidewalk Bluetooth LE deinitialization and reinitialization cycle, advertising may fail to restart.
  When a Bluetooth LE connection is established, the Bluetooth stack stops advertising, but the internal advertising state may remain stale.
  On the next ``sid_start()`` call, ``sid_ble_advert_update()`` might call the Bluetooth HCI API with an invalid advertising set.
  This results in an ``Advertising failed to update (err -22)`` error and causes ``sid_start()`` to return ``SID_ERROR_GENERIC``.

  **Affected platforms:** All platforms.

  **Workaround:** Reset the device after ``sid deinit`` and before calling ``sid init`` again.
  Alternatively, avoid the deinit and reinit cycle unless you perform a full device reset.

.. rst-class:: v1-1-0

KRKNWK-21514: Sidewalk option for Bluetooth LE config (``SID_OPTION_BLE_USER_CONFIG``) is not supported in the NCS
  Calling the Sidewalk Kconfig option ``SID_OPTION_BLE_USER_CONFIG`` causes the application to crash.

  **Affected platforms:** All platforms.

  Fixed in Amazon Sidewalk Add-on v1.2.0.

.. rst-class:: v1-0-1

KRKNWK-19950: SBDT speed degrades when using the v1.18 libraries
  There is a noticeable degradation in the speed of the SBDT when using the version 1.18 of Amazon Sidewalk libraries.

.. rst-class:: v1-0-1

KRKNWK-19948: There is a mismatch in ACK parameters between the ``send`` function and ``on_msg_sent`` JSON output
  The ACK parameters utilized by the ``send`` function do not show correctly in the callback and are incorrectly set in the UART debug logs.

.. rst-class:: v1-0-1

KRKNWK-19119: Multi-image DFU fails to restore after a sample reset
  The multi-image DFU does not successfully restore following a sample reset.

  **Workaround:** If the device is reset during the DFU process, the DFU always restarts from the beginning.

.. rst-class:: v1-0-1

KRKNWK-20330: The Amazon Sidewalk application crashes on startup when the ``CONFIG_NANOPB`` Kconfig option is enabled
  The Amazon Sidewalk libraries include a nanopb implementation that conflicts with the one in Zephyr, causing the application to crash immediately after startup.

  **Workaround:** Disable the ``CONFIG_NANOPB`` Kconfig option and enable the ``CONFIG_SIDEWALK_NANOPB`` Kconfig option instead.
  This configuration uses the nanopb implementation from the Amazon Sidewalk libraries, allowing the application to start successfully.

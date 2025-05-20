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

List of known issues for v1.0.0
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

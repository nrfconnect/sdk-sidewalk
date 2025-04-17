.. _sidewalk_release_notes_addon_v100:

Release notes for Amazon Sidewalk Add-On v1.0.0
###############################################

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

This is an initial release of the Amazon Sidewalk Add-on.
Starting with the nRF Connect SDK release v3.0.0, the Amazon Sidewalk protocol is no longer a part of the nRF Connect SDK.

* Added:

  * Latest integration of Amazon Sidewalk v1.18 libraries.
  * A new :ref:`migration_guide_addon_v010` page, detailing setup and migration instructions for Amazon Sidewalk add-on.
  * Disable option for the state notifier.
  * New `Zephyr Memory Storage (ZMS)`_ back-end integration for `Settings subsystem`_ available in the release v3.0.0 of the nRF Connect SDK.

    .. note::
      The new implementation of ZMS is not compatible with the previous version.

* Updated:

  * Amazon Sidewalk to a west manifest application.
    It is now a part of the `nRF Connect SDK Add-ons`_.
  * Optimized the SubGHz configuration parameters to enhance compatibility with the new Amazon Sidewalk libraries.
  * Kconfig options to align them with the changes in the nRF Connect SDK.
  * Disabled the RTT backend.
    Logs and shell are now available on UART only.
  * Increased a log thread delay.
  * The default bootloader signature alghoritm.
    It has been changed from RSA (``SB_CONFIG_BOOT_SIGNATURE_TYPE_RS``) to ECDSA 512 (``SB_CONFIG_BOOT_SIGNATURE_TYPE_ECDSA_P512``).

    .. note::
      Note that the DFU can only occur when the algorithm matches.
      Therefore, to enable DFU, ensure you have set the ``SB_CONFIG_BOOT_SIGNATURE_TYPE_RS`` Kconfig option.

.. _sidewalk_release_notes_addon_changelog:

Release notes for Amazon Sidewalk Add-On (TBD)
##############################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

* Removed:

  * Support for the nRF52 Series (including the nRF52840 DK).
  * Support for the nRF53 Series (including the nRF5340 DK and Thingy:53).

* Updated:

  * MCUboot signature type to follow the recommended defaults in the nRF Connect SDK.
    On nRF54L Series platforms, the bootloader now uses **ED25519** instead of **RSA**.
    This reduces MCUboot boot time compared to RSA-based verification.
  * MCUboot configuration to align with NCS recommendations: picolibc and link-time optimization (LTO).

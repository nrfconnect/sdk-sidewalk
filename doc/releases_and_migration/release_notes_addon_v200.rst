.. _sidewalk_release_notes_addon_v200:

Release notes for Amazon Sidewalk Add-On v2.0.0
###############################################

.. contents::
   :local:
   :depth: 2

This page tracks changes and updates in Amazon Sidewalk Add-On v2.0.0-rc1 as compared to v1.3.0.
For more information refer to the following section.

For the list of potential issues, see the `known issues page on the main branch`_. 

Changelog
*********

* Added:

  * The ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_RELEASE`` (default) and ``CONFIG_SIDEWALK_PREBUILT_LIBRARIES_DEBUG`` Kconfig options for selecting the release or debug variant of the prebuilt Sidewalk libraries.
  * Regulatory region configuration in the Sidewalk platform initialization, required by the Sidewalk 2.0 libraries to create the LoRa and FSK MACs.
  * ``send_levels``, ``send_pseudorange`` and ``manage_effort`` commands in the location shell of the Sidewalk end device sample.
  * Support for the ``request_fast_adv`` Bluetooth LE adapter interface, which forces the fast advertising phase on an external discoverability request.

* Updated:

  * The nRF Connect SDK from v3.4.0 to v3.4.1.
  * The Amazon Sidewalk libraries to the 2.0.0.60 release.
  * Sidewalk end device sample DUT shell to support QA workflows.

* Fixed:

  * An issue where Bluetooth LE connection mutex handling and advertising during an active Bluetooth LE connection did not work correctly.
  * An issue where restarting Bluetooth LE advertising (for example, on Sidewalk ID rotation) resets the fast advertising phase.
  
.. _sidewalk_release_notes_addon_v101:

Release notes for Amazon Sidewalk Add-On v1.0.1
###############################################

This page tracks changes and updates as compared to the latest official release.
For more information refer to the following section.

For the list of potential issues, see the :ref:`known_issues` page.

Changelog
*********

This is a bugfix release, with no new features or backward compatibility issues.

* Updated:
  * Fixed the Bluetooth LE MAC rotation issue by changing it from static to private.
  * Added a workaround for the issue with the nanopb implementation.
    For details, see the KRKNWK-20330 issue description on the :ref:`known_issues` page.
  * Removed registration tools, unnecessary since Sidewalk Gateway provides automatic registration using FFN.

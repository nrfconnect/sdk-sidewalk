.. _migration_guide_addon_v010:

Migration guide for moving to Sidewalk Add-on
#############################################

Starting with the v3.0.0 release of the `nRF Connect SDK`_, a new standalone :file:`west.yaml` setup has been introduced.
Follow this guide to migrate your Sidewalk development environment from using the nRF Connect SDK manifest setup.

Overview
********

The Sidewalk repository has been moved out of the NCS manifest into its own :file:`west.yml` configuration.
This change provides more flexibility and better control over the Sidewalk development environment.
The Sidewalk release cycle is now separate from the nRF Connect SDK, allowing for faster releases of new libraries.
You can now find Amazon Sidewalk under the `nRF Connect SDK Add-ons`_ index.
This solution enables an alternative setup method using the `nRF Connect for VS Code`_.

Migration steps
***************

To successfully migrate to Amazon Sidewalk Add-on you must choose the migration method.
If you wish to set up a clean repository, follow one of the methods outlined on the :ref:`setting_up_sdk_sidewalk_repository` page.
If you already have an nRF Connect SDK setup with Sidewalk and wish to migrate, you must complete the following steps:

1. Disable the Sidewalk group in the manifest.

   .. code-block:: console

      west config manifest.group-filter -- "-sidewalk"

#. Set Sidewalk as a workspace application repository.

   .. code-block:: console

      west config manifest.path sidewalk


#. Update all repositories.

   .. code-block:: console

      west update


Verification
************

Verify if your migration was successful by completing the following steps:

#. Ensure that all repositories are properly cloned.

   .. code-block:: console

      west list

   Expected output should show all required repositories with their paths and revisions.

   .. code-block:: console

      name                    path                    revision
      ----------------------  ----------------------  ----------------------
      manifest               sidewalk                HEAD
      ...

   An invalid output might include:

   * Empty list
   * Missing repositories
   * Error messages
   * The ``Not found`` status for any of the repositories

#. Check the build process.

   .. parsed-literal::
      :class: highlight

      west build -b *board_target* samples/sid_end_device

   This will build the Sidewalk end device sample application.
   If the build succeeds, your migration was successful.

Troubleshooting
***************

See the list of common issues and resolutions during the migration process.

Repository conflicts
====================

If you encounter repository conflicts, do the following:

#. Remove the :file:`.west` directory.

   .. code-block:: console

      rm -rf .west

#. Reinitialize west.

   .. code-block:: console

      west init -l sidewalk

#. Update all repositories.

   .. code-block:: console

      west update

Resolving toolchain mismatch
============================

Execute the following commands to get the toolchain hash and download the necessary toolchain:

   .. code-block:: console

      nrf/scripts/toolchain.py
      nrfutil toolchain install --ncs-version $(nrf/scripts/toolchain.py --ncs-version)


Build errors
============

If you encounter build errors, ensure the following:

 * All dependencies are installed.
 * NCS environment is sourced correctly.
 * You are using the correct board target.
 * Your build environment is functioning correctly.
   To do this, build the `Zephyr's Hello World`_ sample.

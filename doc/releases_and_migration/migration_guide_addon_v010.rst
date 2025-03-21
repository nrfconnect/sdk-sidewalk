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

Prerequisites
*************

Before you begin the migration process, you must have the following tools installed:

* Git
* Python 3.8 or later
* The `west`_ tool
* `nRF Util`_

Migration steps
***************

To successfully migrate to Amazon Sidewalk Add-on you must choose the migration method.
If you wish to set up a clean repository, follow one of the methods outlined on the :ref:`setting_up_sdk_sidewalk_repository` page.
If you already have an nRF Connect SDK setup with Sidewalk and wish to migrate, you must complete the following steps:

#. Remove the :file:`.west` directory to clean the west configuration.

   .. code-block:: console

      rm -rf .west

#. Checkout and pull main branch in the Sidewalk repository.

   .. code-block:: console

      cd sidewalk
      git checkout main
      git pull origin main
      cd ..

#. Initialize west with local manifest.

   .. code-block:: console

      west init -l sidewalk

#. Update all repositories, by running the following command:

   .. code-block:: console

      west update

   Depending on your connection, the update might take some time.

#. Execute the following commands to get the toolchain hash and download the necessary toolchain:

   .. code-block:: console

      nrf/scripts/toolchain.py
      nrfutil toolchain install --ncs-version $(nrf/scripts/toolchain.py --ncs-version)

#. Install the required Python packages for both nRF Connect SDK and Zephyr by running the following commands:

   .. code-block:: console

      pip install -r nrf/scripts/requirements.txt
      pip install -r zephyr/scripts/requirements.txt

#. Install Python dependencies.

   .. code-block:: console

      pip install -r requirements.txt

Verification
************

Verify if your migration was successful by completing the following steps:

1. Ensure that all repositories are properly cloned.

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

Build errors
============

If you encounter build errors, ensure the following:

 * All dependencies are installed.
 * NCS environment is sourced correctly.
 * You are using the correct board target.
 * Your build evironment is functioning correctly.
   To do this, build the `Zephyr's Hello World`_ sample.

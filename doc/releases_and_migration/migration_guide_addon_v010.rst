.. _migration_guide_addon_v010:

Migration Guide for moving to Sidewalk Add-on 
############################################

This guide explains how to migrate your Sidewalk development environment from using Sidewalk in nRF Connect SDK to Sidewalk Add-on.

Overview
========

The Sidewalk repository is now distributed as a separate add-on repository, using workspace application repository workflow. 
The Sidewalk release cycle will be detached from NCS, allowing to always release new libraries on stable versions of NCS.
The Sidewalk repository is now visible in the NCS add-on index repository, which enables an alternative setup method using the VSCode nRF Connect add-on.

Migration Steps
===============

1. Disable Sidewalk group in the manifest

   .. code-block:: console

      west config manifest.group-filter -- "-sidewalk"

#. Use Sidewalk as a workspace application repository

   .. code-block:: console

      west config manifest.path sidewalk


#. Update Repositories

   .. code-block:: console

      west update


Verification
===========

To verify your migration was successful:

1. Check that all repositories are properly cloned:
   
   .. code-block:: console

      west list

   Expected output should show all required repositories with their paths and revisions. For example:

   .. code-block:: console

      name                    path                    revision
      ----------------------  ----------------------  ----------------------
      manifest               sidewalk                HEAD
      ...

   Invalid output would be:
   * Empty list
   * Missing repositories
   * Error messages
   * "Not found" status for any repository

2. Verify the build process:
   
   .. code-block:: console

      west build -b nrf54l15dk/nrf54l15/cpuapp samples/sid_end_device

   This will build the Sidewalk end device sample application. If the build succeeds, your migration was successful.

Common Issues and Solutions
=========================

Repository Conflicts
--------------------

If you encounter repository conflicts during the migration:
   
1. Remove the .west directory:

   .. code-block:: console

      rm -rf .west

2. Reinitialize west:

   .. code-block:: console

      west init -l sidewalk

3. Update repositories:

   .. code-block:: console

      west update

Build Errors
------------

If you encounter build errors:

* Ensure all dependencies are installed
* Check that the NCS environment is properly sourced
* Verify that you're using the correct board target
* Try building the Zephyr hello world sample to verify your basic build environment:
   
   .. code-block:: console

      west build -b nrf54l15dk/nrf54l15/cpuapp zephyr/samples/basic/hello_world

Additional Resources
====================

* :ref:`setting_up_environment`
* :ref:`samples_list`
* :ref:`compatibility_matrix` 

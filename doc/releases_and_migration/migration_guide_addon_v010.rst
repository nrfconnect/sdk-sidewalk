.. _migration_guide_addon_v010:

Migration Guide for moving to Sidewalk Add-on 
############################################

This guide explains how to migrate your Sidewalk development environment from using the NCS manifest to the new standalone west.yml setup.

Overview
========

The Sidewalk repository has been moved out of the NCS manifest into its own west.yml configuration. This change provides more flexibility and better control over the Sidewalk development environment. The Sidewalk release cycle will be detached from NCS, allowing for faster releases of new libraries on stable versions of NCS. The Sidewalk repository is now visible in the NCS add-on index repository, which enables an alternative setup method using the VSCode NRF Connect add-on.

Prerequisites
============

Before starting the migration, ensure you have:

* Git installed on your system
* Python 3.8 or later installed
* The west tool installed
* nrfutil installed

Migration Steps
=============

1. Initialize Sidewalk Repository
-------------------------------

There are three ways to initialize the Sidewalk repository:

.. tabs::

   .. tab:: Option 1: Direct Initialization (Recommended)

      .. code-block:: console

         # Initialize west with the remote manifest
         west init -m https://github.com/nrfconnect/sdk-sidewalk

   .. tab:: Option 2: Manual Clone and Initialize

      .. code-block:: console

         # Clone the Sidewalk repository into sidewalk directory
         git clone https://github.com/nordicsemiconductor/sidewalk.git sidewalk
         # Initialize west with local manifest
         west init -l sidewalk

   .. tab:: Option 3: Clean Existing Configuration

      If you already have an NCS setup with Sidewalk, you need to remove the existing west configuration first:

      .. code-block:: console

         # Remove the .west directory to clean the west configuration
         rm -rf .west
         # Checkout and pull main branch in Sidewalk repository
         cd sidewalk
         git checkout main
         git pull origin main
         cd ..
         # Initialize west with local manifest
         west init -l sidewalk

2. Update Repositories
---------------------

   .. code-block:: console

      # Update all repositories
      west update

3. Update Toolchain and Install Dependencies
------------------------------------------

   .. code-block:: console

      # Get the toolchain hash and download it
      nrf/scripts/toolchain.py
      nrfutil toolchain install --ncs-version $(nrf/scripts/toolchain.py --ncs-version)
      # Install Python dependencies for nRF and Zephyr
      pip install -r nrf/scripts/requirements.txt
      pip install -r zephyr/scripts/requirements.txt
      # Install Sidewalk Python requirements
      pip install -r requirements.txt

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

1. Repository Conflicts
----------------------

   If you encounter repository conflicts during the migration:

   .. code-block:: console

      # Remove the .west directory
      rm -rf .west
      # Reinitialize west
      west init -l sidewalk
      # Update repositories
      west update

2. Build Errors
--------------

   If you encounter build errors:

   * Ensure all dependencies are installed
   * Check that the NCS environment is properly sourced
   * Verify that you're using the correct board target
   * Try building the Zephyr hello world sample to verify your basic build environment:
     
     .. code-block:: console

        west build -b nrf54l15dk/nrf54l15/cpuapp zephyr/samples/basic/hello_world

Additional Resources
==================

* :ref:`setting_up_environment`
* :ref:`samples_list`
* :ref:`compatibility_matrix` 

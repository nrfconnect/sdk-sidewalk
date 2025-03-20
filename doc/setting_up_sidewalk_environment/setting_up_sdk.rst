.. _setting_up_dk:

Setting up the SDK
##################

.. contents::
   :local:
   :depth: 2

See the compatibility between the versions of Amazon Sidewalk and the nRF Connect SDK by referring to the :ref:`compatibility_matrix`.
Once confirmed, follow the `Installing the nRF Connect SDK`_ instructions.

.. _dk_building_sample_app:

Setting up the Sidewalk Repository
***********************************

The Sidewalk repository is now managed through its own west.yml configuration. To set up the Sidewalk development environment:
There are two ways to initialize the Sidewalk repository:

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

.. note::
   If you are migrating from an existing NCS setup with Sidewalk, please refer to the :ref:`migration_guide_addon_v010` for detailed migration steps.

4. Update all repositories:

   .. code-block:: console

      west update

   Depending on your connection, the update might take some time.

5. Install the toolchain and update Python packages:

   .. code-block:: console

      # Get the toolchain hash and download it
      nrf/scripts/toolchain.py
      nrfutil toolchain install --ncs-version $(nrf/scripts/toolchain.py --ncs-version)
      # Install Python dependencies for nRF and Zephyr
      pip install -r nrf/scripts/requirements.txt
      pip install -r zephyr/scripts/requirements.txt
      # Install Sidewalk Python requirements
      pip install -r requirements.txt

Extracting nRF Command Line Tools
*********************************

Download the nRF Command Line from the `nRF command line tools`_ page.

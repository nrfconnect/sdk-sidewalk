.. _setting_up_sdk:

Setting up the SDK
##################

.. contents::
   :local:
   :depth: 2

See the compatibility between the versions of Amazon Sidewalk and the nRF Connect SDK by referring to the :ref:`compatibility_matrix`.
Once confirmed, follow the `Installing the nRF Connect SDK`_ instructions.

.. _setting_up_sdk_sidewalk_repository:

Setting up the Sidewalk repository
***********************************

The Sidewalk repository is managed through its own :file:`west.yml` configuration.

1. Initialize the Sidewalk repository, using one of the following methods:

   .. tabs::

      .. tab:: Direct initialization (Recommended)

         a. Initialize west with the remote manifest.

            .. code-block:: console

               west init -m https://github.com/nrfconnect/sdk-sidewalk

      .. tab:: Manual cloning and initialization

         a. Clone the Sidewalk repository into the sidewalk directory.

            .. code-block:: console

               git clone https://github.com/nordicsemiconductor/sidewalk.git sidewalk

         #. Initialize west with local manifest.

            .. code-block:: console

               west init -l sidewalk

   .. note::
      If you are migrating from an existing nRF Connect SDK setup with Sidewalk, refer to the :ref:`migration_guide_addon_v010` for detailed steps.

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

Extracting nRF Command Line Tools
*********************************

Download the nRF Command Line from the `nRF command line tools`_ page.

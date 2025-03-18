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

1. Clone the Sidewalk repository:

   .. code-block:: console

      git clone https://github.com/nordicsemiconductor/sidewalk.git sidewalk

2. If you are migrating from an existing NCS setup with Sidewalk, clean the west configuration:

   .. code-block:: console

      # Remove the .west directory to clean the west configuration
      rm -rf .west
      # Checkout and pull main branch in Sidewalk repository
      cd sidewalk
      git checkout main
      git pull origin main
      cd ..

3. Initialize west with the new manifest:

   .. code-block:: console

      west init -l sidewalk

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

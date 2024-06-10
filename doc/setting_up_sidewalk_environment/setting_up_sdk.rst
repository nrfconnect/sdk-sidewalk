.. _setting_up_dk:

Setting up the SDK
##################

.. contents::
   :local:
   :depth: 2

See the compatibility between the versions of Amazon Sidewalk and the nRF Connect SDK by referring to the :ref:`compatibility_matrix`.
Once confirmed, follow the `Installing the nRF Connect SDK`_ instructions.

.. _dk_building_sample_app:

Downloading the Sidewalk repository
***********************************

Once you have installed the nRF Connect SDK, download the Sidewalk application:

1. Open a terminal window in the ncs folder.
   Your directory structure should look as follows:

   .. code-block:: console

       .
       |___ .west
       |___ bootloader
       |___ modules
       |___ nrf
       |___ nrfxlib
       |___ zephyr
       |___ ...

#. Enable the Sidewalk group filter for west.

   .. code-block:: console

      $ west config manifest.group-filter "+sidewalk"

   Check for Sidewalk presence in west:

   .. code-block:: console

      $ west list sidewalk
      sidewalk     sidewalk                     <sidewalk_revision> https://github.com/nrfconnect/sdk-sidewalk

#. Update all repositories:

   .. code-block:: console

      $ west update

   Depending on your connection, the update might take some time.

#. Install Python requirements for Sidewalk.

   .. code-block:: console

      $ pip install -r sidewalk/requirements.txt

Extracting nRF Command Line Tools
*********************************

Download the nRF Command Line from the `nRF command line tools`_ page.

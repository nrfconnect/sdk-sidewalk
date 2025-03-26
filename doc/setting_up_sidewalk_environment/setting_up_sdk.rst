.. _setting_up_sdk:

Setting up the SDK
##################

.. contents::
   :local:
   :depth: 2

See the compatibility between the versions of Amazon Sidewalk and the nRF Connect SDK by referring to the :ref:`compatibility_matrix`.

Install the nRF Connect SDK
***************************

To work with the Sidewalk add-on, you need to install the nRF Connect SDK, including all its prerequisites and the nRF Connect SDK toolchain.
Follow the `Installing the nRF Connect SDK`_ instructions, but instead of point 4. Get the nRF Connect SDK code, do :ref:`setting_up_sdk_sidewalk_repository` (described below).


.. _setting_up_sdk_sidewalk_repository:

Get the Sidewalk Add-on code
****************************

The Sidewalk Add-on is distributed as a Git repository, and is managed through its own west manifest.
The compatible nRF Connect SDK version is specified in the :file:`west.yml` file.

.. tabs::

   .. group-tab:: nRF Connect for Visual Studio Code

      To clone the Sidewalk Add-on code, together with compatible nRF Connect SDK, complete the following steps:

      1. Open the nRF Connect extension in Visual Studio Code by clicking its icon in the :guilabel:`Activity Bar`.
      #. In the extension's :guilabel:`Welcome View`, click on :guilabel:`Create a new application`.
         The list of actions appears in the Visual Studio Code's quick pick.
      #. Click :guilabel:`Browse nRF Connect SDK Add-on Index`.
         The list of available nRF Connect SDK Add-ons appears in the Visual Studio Code's quick pick.
      #. Select :guilabel:`Amazon Sidewalk Add-on`.
      #. Select the Add-on version to install.

      The Add-on and compatible nRF Connect SDK installation starts and it can take several minutes.

   .. group-tab:: Command line

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

      #. Update all repositories, by running the following command:

         .. code-block:: console

            west update

         Depending on your connection, the update might take some time.

      #. Install Python dependencies.

         .. code-block:: console

            pip install -r sidewalk/requirements.txt

To create an application, use Sidewalk: sid_end_device sample hello app as a starting point.

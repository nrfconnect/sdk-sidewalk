.. _setting_up_dk:

Setting up the SDK
##################

See the compatibility between the versions of Amazon Sidewalk and the nRF Connect SDK by referring to the :ref:`compatibility_matrix`.
Once confirmed, follow one of the installation methods below:

.. tabs::

   .. tab:: Automatic installation (Toolchain Manager)

      Follow the steps in `Installing automatically`_ to perform automatic installation using the Toolchain Manager.

   .. tab:: Manual installation

      Follow the steps in `Installing manually`_ to perform a manual installation.

   .. tab:: VSCode Dev Container (deprecated)

         .. note::
            Dev Container is not supported any more since the Sidewalk distribution model has changed.
            See :ref:`dk_building_app_in_nrf_manifest` for more details.

.. _dk_building_sample_app:

Downloading the Sidewalk repository
***********************************

The Sidewalk distribution model supports the following download methods.
Choose the right download method depending on the version of sdk-sidewalk that you are using.

  * For sdk-sidewalk ``v1.14.5-dev1`` (and before) and sdk-nrf ``v2.4.1`` (and before), follow the steps in :ref:`dk_building_app_with_manifest`.
  * For any later releases of sdk-sidewalk and sdk-nrf, follow the steps in :ref:`dk_building_app_in_nrf_manifest`.
  * For sdk-sidewalk ``main`` and sdk-nrf ``main``, follow the steps in :ref:`dk_building_app_in_nrf_manifest`.


.. _dk_building_app_in_nrf_manifest:

Sidewalk as an nRF Connect SDK repository
-----------------------------------------

Follow these steps to download the Sidewalk application for the nRF Connect SDK:

1. Open a terminal window.

   If you have installed the nRF Connect SDK through the automatic installation (using the Toolchain Manager), click the down arrow next to the version you installed, and select :guilabel:`Open bash`.

   .. figure:: /images/toolchain_manager.png

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

#. If you have worked with the Sidewalk application in a previous distribution model (as a manifest repository), check the manifest file.

   Make sure the manifest path points to :file:`west.yml` inside the :file:`nrf` directory:

      .. code-block:: console

         $ west manifest --path
         /path-to-ncs-folder/nrf/west.yml

   In case your manifest path points to a different file, use the following command:

      .. code-block:: console

         $ west config manifest.path nrf

#. Enable the Sidewalk group filter for west.

   .. code-block:: console

      $ west config manifest.group-filter "+sidewalk"

   Check for Sidewalk presence in west:

   .. code-block:: console

      $ west list | grep sidewalk
      sidewalk     sidewalk                     <sidewalk_revision> https://github.com/nrfconnect/sdk-sidewalk

#. Update all repositories:

   .. code-block:: console

      $ west update

   Depending on your connection, the update might take some time.

#. Install Python requirements for Sidewalk.

   .. code-block:: console

      $ pip install -r sidewalk/requirements.txt

.. _dk_building_app_with_manifest:

Sidewalk as an application with manifest repository
---------------------------------------------------

Follow these steps to download the Sidewalk application for the nRF Connect SDK:

1. Open a terminal window.

   If you have installed the nRF Connect SDK through the automatic installation (using the Toolchain Manager), click the down arrow next to the version you installed, and select :guilabel:`Open bash`.

   .. figure:: /images/toolchain_manager.png

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

#. Clone the Sidewalk application repository from the `sdk-sidewalk repository`_ to the nRF Connect SDK folder and name it ``sidewalk``:

   .. code-block:: console

      $ git clone --branch <sidewalk_revision> https://github.com/nrfconnect/sdk-sidewalk.git sidewalk

   For example:

   .. code-block:: console

      $ git clone --branch v1.14.4 https://github.com/nrfconnect/sdk-sidewalk.git sidewalk

   .. note::

      This method works for Sidewalk revisions up to ``v1.14.5-dev1``.
      For later versions and for the ``main`` branch, follow the steps in :ref:`dk_building_app_in_nrf_manifest`.

#. Install Python requirements for Sidewalk.

   .. code-block:: console

      $ pip install -r sidewalk/requirements.txt

#. Set the Sidewalk application manifest and update.

   a. Check the current manifest path:

      .. code-block:: console

         $ west manifest --path
         /path-to-ncs-folder/nrf/west.yml

   #. Set the manifest path to the Sidewalk repository:

      .. code-block:: console

         $ west config manifest.path sidewalk

   #. Update all repositories:

      .. code-block:: console

         $ west update

      Depending on your connection, the update might take some time.

   #. Verify the new manifest path:

     .. code-block:: console

        $ west manifest --path
        /path-to-ncs-folder/sidewalk/west.yml


Extracting nRF Command Line Tools
*********************************

To download the nRF Command Line, go to the `nRF command line tools`_ page.

.. include:: ../ncs_links.rst

.. _Jlink tools: https://www.segger.com/downloads/jlink/
.. _nrf tools: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
.. _Docker: https://docs.docker.com/engine/install/ubuntu/
.. _nRF_command_line_tools: https://infocenter.nordicsemi.com/topic/ug_nrf_cltools/UG/cltools/nrf_installation.html
.. _sdk-sidewalk repository: https://github.com/nrfconnect/sdk-sidewalk
.. _Semtech SX1262MB2CAS eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _GNU Arm Embedded Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
.. _nRF Command Line Tools: https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs
.. _Makefile.posix: ../../components/toolchain/gcc/Makefile.posix

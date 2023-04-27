.. _setting_up_dk:

Setting up SDK
##############

The Amazon Sidewalk solution from Nordic Semiconductor is based on the `nRF Connect SDK v2.3.0`_.
In order to create a Sidewalk prototype, you need to use the following devices:

* `nRF52840 DK`_ - for both Bluetooth LE and LoRa or FSK applications.
* `Semtech SX1262MB2CAS eval board`_ - only for LoRa or FSK applications.

You can set up your development environment by following one of the installation methods below:

.. tabs::

   .. tab:: Automatic installation (Toolchain Manager)

      Follow the steps in `Installing automatically`_ to perform automatic installation using the Toolchain Manager.

   .. tab:: Manual installation

      Follow the steps in `Installing manually`_ to perform a manual installation.

   .. tab:: VSCode Dev Container (experimental, Linux only)

         .. note::
            Before enrolling with the development environment, check :file:`sidewalk/.devcontainer/README.md` for known issues and limitations.

      #. Install the ``ms-vscode-remote.remote-containers`` VS Code extension.

      #. Install `Docker`_.

      #. Download the `sdk-sidewalk repository`_.

      #. Open the repository in Dev Container.

         a. In the Visual Studio Code, open the Command Pallete by using the F1 key or the Ctrl-Shift-P key combination.
         #. Run ``Dev Containers: Open Folder in Container...`` command.

         .. note::
            The first launch will take a few minutes to download and set up the docker container.
            Subsequent launches will be almost instant.

      #. In the VS Code terminal, run the bootstrap script :file:`sidewalk/.devcontainer/bootstrap.sh`.

         .. note::
            The bootstrap script will update all west modules.
            This action may take a few minutes.

For additional information on setting up the device and Nordic's development environment and tools, see the `nRF Connect SDK Getting started`_ guide.

.. _dk_building_sample_app:

Downloading Sidewalk repository
*******************************

Download the Sidewalk application for the nRF Connect SDK:

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

#. Clone the Sidewalk application repository from the `sdk-sidewalk repository`_ to the nRF Connect SDK folder, and name it ``sidewalk`` by running the following command: 

   .. code-block:: console

      $ git clone https://github.com/nrfconnect/sdk-sidewalk.git sidewalk

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

.. _nRF Connect SDK v2.3.0: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/index.html
.. _Jlink tools: https://www.segger.com/downloads/jlink/
.. _nrf tools: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
.. _Zephyr toolchain: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_installing.html#install-a-toolchain
.. _Installing automatically: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_assistant.html#installing-automatically
.. _Installing manually: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_installing.html#install-the-required-tools
.. _Docker: https://docs.docker.com/engine/install/ubuntu/
.. _nRF_command_line_tools: https://infocenter.nordicsemi.com/topic/ug_nrf_cltools/UG/cltools/nrf_installation.html
.. _sdk-sidewalk repository: https://github.com/nrfconnect/sdk-sidewalk
.. _nRF Connect SDK Getting started: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started.html
.. _nRF52840 DK: https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262MB2CAS eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _GNU Arm Embedded Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
.. _nRF Command Line Tools: https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs
.. _Makefile.posix: ../../components/toolchain/gcc/Makefile.posix

.. _setting_up_dk:

Setting up SDK
##############

The Amazon Sidewalk solution from Nordic Semiconductor is based on `nRF Connect SDK v.2.3.0`_.
The following devices are used throughout this guide in order to create a Sidewalk prototype.

* `Nordic nRF52840-DK`_ - for both Bluetooth LE and LoRa or FSK applications.
* The `Semtech SX1262MB2CAS eval board`_ - only for LoRa or FSK applications.

You can set up your development environment by following one of the installation methods below:

.. tabs::

   .. tab:: Automatic installation (Toolchain Manager)

      Complete the automatic installation by following the `Installing automatically`_ guide.

   .. tab:: Manual installation

      Complete the manual installation by following the `Installing manually`_ guide.

   .. tab:: VSCode Dev Container (experimental, Linux only)

         .. note::
            Before enrolling with development environment, check :file:`sidewalk/.devcontainer/README.md` for known issues and limitations.

      #. Install the ``ms-vscode-remote.remote-containers`` VS Code extension.

      #. Install `Docker`_.

      #. Download the `nRF Sidewalk Repository`_.

      #. Open the repository in devcontainer.

         a. In VS Code, open the Command Pallete by using the F1 key or the Ctrl-Shift-P key combination.
         #. Type ``Dev Containers: Open Folder in Container...`` command.

         .. note::
            The first launch will take a few minutes to download and set up the docker container.
            Subsequent launches will be almost instant.

      #. In VS Code terminal, run the bootstrap script :file:`sidewalk/.devcontainer/bootstrap.sh`.

         .. note::
            The bootstrap script will update all west modules.
            This action may take a few minutes.

For additional information regarding setting up the device and familiarizing yourself with the Nordic's development environment and tools, see the `NCS Getting started`_ guide.

.. _dk_building_sample_app:

Downloading Sidewalk repository
*******************************

#. Download Sidewalk application for nRF Connect SDK.

   * Clone Sidewalk application repository from `nRF Sidewalk Repository`_ to the NCS folder and name it `sidewalk` (private repository, special access required).

      .. code-block:: console

         $ cd $ZEPHYR_BASE/../
         $ git clone https://github.com/nrfconnect/sdk-sidewalk.git sidewalk

   * Install Python requirements for Sidewalk.

       .. code-block:: console

         $ pip install -r sidewalk/requirements.txt

   * Use application manifest and update.

      .. code-block:: console

         $ west config manifest.path sidewalk
         $ west update

Extracting nRF Command Line Tools
*********************************

To download nRF Command Line, go to `nRF command line tools`_ page.

.. _nRF Connect SDK v.2.3.0: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/index.html
.. _Jlink tools: https://www.segger.com/downloads/jlink/
.. _nrf tools: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
.. _Zephyr toolchain: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_installing.html#install-a-toolchain
.. _Installing automatically: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_assistant.html#installing-automatically
.. _Installing manually: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/gs_installing.html#install-the-required-tools
.. _Docker: https://docs.docker.com/engine/install/ubuntu/
.. _nRF_command_line_tools: https://infocenter.nordicsemi.com/topic/ug_nrf_cltools/UG/cltools/nrf_installation.html
.. _nRF Sidewalk Repository: https://github.com/nrfconnect/sdk-sidewalk
.. _NCS getting started: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/nrf/getting_started.html
.. _Nordic nRF52840-DK: https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262MB2CAS eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _GNU Arm Embedded Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
.. _nRF Command Line Tools: https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs
.. _Makefile.posix: ../../components/toolchain/gcc/Makefile.posix
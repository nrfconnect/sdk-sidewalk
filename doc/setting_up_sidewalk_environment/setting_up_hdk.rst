.. _setting_up_dk:

Setting up hardware development kit
###################################

The following devices are used throughout this guide in order to create a Sidewalk prototype.

* `Nordic nRF52840-DK`_ - for both Bluetooth LE and LoRa/FSK applications.
* `Semtech SX1262 mbed shield eval board`_ - only for LoRa/FSK applications.

You can set up your device by following one of the installation methods below:

.. tabs::

   .. tab:: Automatic installation (Toolchain Manager)

      Complete the automatic installation by following the `Toolchain Manager`_ installation guide.

   .. tab:: Manual Installation

      Manually install the listed tools to build and flash the nRF Connect SDK applications:

      * `Jlink tools`_ to interract with debugger embedded on Development board
      * `nrf tools`_ to flash an image on a board
      * `Zephyr toolchain`_ to build application for development board

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

For additional information regarding setting up the device and familiarizing yourself with the Nordic's development environment and tools, see the `Getting started`_ guide.

.. _dk_building_sample_app:

Building sample application and flash certificates
**************************************************

#. Install nRF Connect SDK with its dependencies.

   * Install `Git LFS`_ via the following command:

      .. code-block:: console

         sudo apt install git-lfs

   * Refer to `NCS getting started`_ documentation for getting started with the nRF Connect SDK.

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

Extracting SDK and generating certificate
*****************************************

#. Place the nRF Connect for Sidewalk package in any directory.
#. Open a terminal in the selected directory and issue the following commands:

   .. code-block:: console

      # NCS_FOR_SIDEWALK_SDK_PATH should point to the directory SDK package is placed (see above)
      $ export NCS_FOR_SIDEWALK_SDK_PATH=${ZEPHYR_BASE}/../sidewalk

#. Use the Sidewalk certificate generation tool provided within the SDK tools folder to generate your application server certificates.

   .. note::
       For instructions on cloud configuration, see :ref:`configuring_aws_cloud`.

   a. Navigate to the script within the Sidewalk tools folder of the SDK.

      .. code-block:: console

         $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/tools/application_server_cert

   #. Install the requirements.

      .. code-block:: console

         $ pip3 install --user -r requirements.txt

   #. Run the application key generation script to generate your application server certificates.

      .. code-block:: console

         $ python3 generate_application_server.py --app_srv_serial <ApplicationServerSerialNumber> \
         --apid <APID>

      .. note::

          * ApplicationServerSerialNumber is a 4-byte, base10 integer (0 to 4294967295) that you define.
          * APID consists of 4-digit alphanumeric value.
            For now you can assign any value.

      .. code-block:: console

         # Sample command:
         $ python3 generate_application_server.py --app_srv_serial 1234567890 --apid cafe
         $ ls app-server*

         #  Sample output:
         app-server-ed25519-csr.bin
         app-server-ed25519.private.bin
         app-server-p256r1-signature.bin
         app-server-ed25519-private.pem
         app-server-ed25519.public.bin
         app-server-p256r1-signature.der
         app-server-ed25519-private.txt
         app-server-p256r1-csr.bin
         app-server-p256r1.c
         app-server-ed25519-public.pem
         app-server-p256r1-private.pem
         app-server-p256r1.private.bin
         app-server-ed25519-public.txt
         app-server-p256r1-private.txt
         app-server-p256r1.public.bin
         app-server-ed25519-signature.bin
         app-server-p256r1-public.pem
         app-server-ed25519.c
         app-server-p256r1-public.txt

#. Copy the application server public key to the provisioning tool folder, which will be used for your specific hardware during provisioning and configuring your Sidewalk Endpoints.

   .. code-block:: console

      $ cp app-server-ed25519.public.bin ../provision/

   .. note::
       The script will output several keys.
       Two of them are required.

       * :file:`app-server-ed25519-private.txt` - an input to configure your AWS cloud in :ref:`configuring_aws_cloud`.
       * :file:`app-server-ed25519.public.bin` - an input to :file:`provision.py` script as the application server public key.



.. _Jlink tools: https://www.segger.com/downloads/jlink/
.. _nrf tools: https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools/download
.. _Zephyr toolchain: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#install-a-toolchain
.. _Toolchain Manager: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_assistant.html#id5
.. _Docker: https://docs.docker.com/engine/install/ubuntu/
.. _nRF_command_line_tools: https://infocenter.nordicsemi.com/topic/ug_nrf_cltools/UG/cltools/nrf_installation.html
.. _nRF Sidewalk Repository: https://github.com/nrfconnect/sdk-sidewalk
.. _Git LFS: https://git-lfs.github.com/
.. _NCS getting started: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html
.. _Nordic nRF52840-DK: https://www.nordicsemi.com/Software-and-tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262 mbed shield eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _Getting started: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/getting_started.html
.. _GNU Arm Embedded Toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
.. _nRF Command Line Tools: https://www.nordicsemi.com/Software-and-Tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs
.. _Makefile.posix: ../../components/toolchain/gcc/Makefile.posix
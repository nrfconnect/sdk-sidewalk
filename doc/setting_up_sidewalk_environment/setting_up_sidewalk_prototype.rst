.. _setting_up_sidewalk_prototype:

Setting up your Sidewalk prototype
##################################

.. contents::
   :local:
   :depth: 2

This section details the prototyping flow for onboarding your Sidewalk devices.
In this flow, you will provision devices individually, allowing you to learn more about prototyping your application.

Onboarding
**********

Before your device can join the Sidewalk network, you must register it in the AWS IoT console and download the credentials it needs to authenticate.
Ensure you have an AWS account.
Complete the `Onboarding your Sidewalk devices`_ steps described in the Amazon Sidewalk documentation.
As a result, you will have a device JSON file named :file:`certificate.json`, or two JSON files named :file:`wireless_device.json` and :file:`device_profile.json`.

Provisioning
************

The credentials you downloaded from the AWS IoT console must now be written to the manufacturing data partition (``mfg_storage``) on the device, which is what allows it to join the Sidewalk network.
Do this with the ``west sid provision`` command, which handles a connected board in a single step.
You can also run the individual steps manually.

Provisioning with west command
==============================

The ``west sid provision`` command automates the following steps:

#. Reads the ``mfg_storage`` partition address from the application build output.
#. Generates a manufacturing HEX file by calling the :file:`provision.py` script from the :file:`sidewalk/tools/provision` directory of the Sidewalk Add-On repository (`sdk-sidewalk`_).
   For details on the script and the manufacturing data format, see `Provision your Sidewalk endpoint and flash the binary image`_.
#. Flashes the generated HEX file to the connected board using `nRF Util`_.

To provision your device:

#. Run ``west sid provision`` with the Sidewalk application build directory and your onboarded device JSON files as arguments.

   * If you are using the combined device JSON file obtained from the AWS IoT console, pass it with ``-C``/``--certificate-json``:

     .. code-block:: console

        west sid provision -d *build_directory* -C certificate.json

   * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, pass both with ``-W``/``--wireless-device-json`` and ``-D``/``--device-profile-json``:

     .. code-block:: console

        west sid provision -d *build_directory* -W wireless_device.json -D device_profile.json

   If multiple boards are connected, the command prompts you to select the target device.

Provisioning with manual steps
==============================

The :file:`provision.py` script required for provisioning is located in the :file:`sidewalk/tools/provision` directory of the Sidewalk Add-On repository (`sdk-sidewalk`_).

.. note::
   You can use the ``--output_hex`` parameter to specify a custom name for the output hex file.
   This change is optional and does not affect the file's compatibility with other supported boards.
   If the parameter is not provided, the default name will be :file:`nordic_aws_nrf52840.hex`.

.. tabs::

   .. group-tab:: nRF52 DKs

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0xFF000

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --wireless_device_json wireless_device.json --device_profile_json device_profile.json --addr 0xFF000

      #. Flash the generated :file:`nordic_aws_nrf52840.hex` file with the provisioning data:

         .. code-block:: console

            nrfjprog --sectorerase --program nordic_aws_nrf52840.hex --reset

   .. group-tab:: nRF54L10

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The nRF54L10 and nRF54LV10 DKs share the same ``mfg_storage`` layout: 4 KB located at the end of the application core RRAM.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0xFC000 --output_hex nordic_aws_nrf54l10.hex

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --wireless_device_json wireless_device.json --device_profile_json device_profile.json --addr 0xFC000 --output_hex nordic_aws_nrf54l10.hex

      #. Flash the generated file with the provisioning data:

         .. code-block:: console

            nrfutil device program --x-family nrf54l --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ --traits jlink --firmware nordic_aws_nrf54l10.hex

   .. group-tab:: nRF54LV10 DK

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The nRF54LV10 DK uses the same ``mfg_storage`` address as the nRF54L10 DK (4 KB at the end of the 1012 KB application core RRAM).

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0xFC000 --output_hex nordic_aws_nrf54lv10.hex

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --wireless_device_json wireless_device.json --device_profile_json device_profile.json --addr 0xFC000 --output_hex nordic_aws_nrf54lv10.hex

      #. Flash the generated file with the provisioning data:

         .. code-block:: console

            nrfutil device program --x-family nrf54l --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ --traits jlink --firmware nordic_aws_nrf54lv10.hex

   .. group-tab:: nRF54L15

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The default address of the :file:`mfg.hex` file provided in the official Amazon Sidewalk documentation is incompatible with the nRF Connect SDK applications.
         To fix it, you must add an argument to the :file:`provision.py` script in order to generate the :file:`mfg.hex` file that is compatible with the nRF Connect SDK memory map.
         Depending on the device, the argument will differ.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0x17c000 --output_hex nordic_aws_nrf54l15.hex

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --wireless_device_json wireless_device.json --device_profile_json device_profile.json --addr 0x17c000 --output_hex nordic_aws_nrf54l15.hex

      #. Flash the generated file with the provisioning data:

         .. code-block:: console

            nrfutil device program --x-family nrf54l --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ --traits jlink --firmware nordic_aws_nrf54l15.hex

   .. group-tab:: nRF54LM20

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The default address of the :file:`mfg.hex` file provided in the official Amazon Sidewalk documentation is incompatible with the nRF Connect SDK applications.
         To fix it, you must add an argument to the :file:`provision.py` script in order to generate the :file:`mfg.hex` file that is compatible with the nRF Connect SDK memory map.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0x1de000 --output_hex nordic_aws_nrf54lm20.hex

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --wireless_device_json wireless_device.json --device_profile_json device_profile.json --addr 0x1de000 --output_hex nordic_aws_nrf54lm20.hex

      #. Flash the generated file with the provisioning data:

         .. code-block:: console

            nrfutil device program --x-family nrf54l --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ --traits jlink --firmware nordic_aws_nrf54lm20.hex

Device factory reset
====================

If you reprovision a device already operating, deregister the previously flashed device credentials by performing a factory reset:

.. tabs::

   .. group-tab:: nRF52 DKs

      Long press **Button 1**.

   .. group-tab:: nRF54L Series DKs

      Long press **Button 0**.

This allows you to register the device in the Sidewalk network with the newly provisioned credentials.

Add MQTT to destination
***********************

#. Log in to `AWS`_.
   Open the AWS IoT Core service.

   .. figure:: /images/AWSIoTCore.png

#. Navigate to :guilabel:`Manage → LPWAN devices → Destinations`.

   .. figure:: /images/AWSIoTCoreDestinations.png

#. Select your destination end click :guilabel:`Edit`.

   .. figure:: /images/AWSIoTCoreDestinationEdit.png

#. Edit the destination details.

    a. Select :guilabel:`Publish to AWS IoT Core message broker`.
    #. Type your MQTT Topic name.

       .. figure:: /images/AWSIoTCoreDestinationTestMQTT.png

    #. Edit permissions.
    #. Select :guilabel:`Create a new service role` and click :guilabel:`Save`.

       .. figure:: /images/AWSIoTCoreDestinationTestRole.png

Use MQTT client
***************

#. Open the MQTT test client.
#. Type your MQTT topic in the filter field and click :guilabel:`Subscribe`.

   .. figure:: /images/AWSIoTCoreMQTT.png

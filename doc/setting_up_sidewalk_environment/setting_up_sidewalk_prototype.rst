.. _setting_up_sidewalk_prototype:

Setting up your Sidewalk prototype
##################################

.. contents::
   :local:
   :depth: 2

To correctly set up your Sidewalk device, first you have to onboard it.
Complete the `Onbarding your Sidewalk devices`_ steps described in the Amazon Sidewalk documentation.
This section details the prototyping flow for onboarding your Sidewalk devices.
In this flow, you will provision devices individually, allowing you to learn more about prototyping your application.

Generate provisioning
*********************

The tools required for provisioning are located in the repository (`sdk-nrf`_ and `sdk-sidewalk`_) under the :file:`sidewalk/tools/provision` path.

.. tabs::

   .. group-tab:: nRF52 and nRF53 DKs

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The default address of the :file:`mfg.hex` file provided in the official Amazon Sidewalk documentation is incompatible with the nRF Connect SDK applications.
         To fix it, you must add an argument to the :file:`provision.py` script in order to generate the :file:`mfg.hex` file that is compatible with the nRF Connect SDK memory map.
         Depending on the device, the argument will differ.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0xFF000

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin
               --wireless_device_json wireless_device.json
               --device_profile_json device_profile.json
               --addr 0xFF000

         .. note::
            The default name of the :file:`mfg.hex` file generated from the :file:`provision.py` script is :file:`nordic_aws_nrf52840.hex`.
            You can use the ``--output_hex`` parameter to specify a custom name for the :file:`mfg.hex` file.

      #. Flash the generated :file:`nordic_aws_nrf52840.hex` file with the provisioning data:

         .. code-block:: console

            nrfjprog --sectorerase --program nordic_aws_nrf52840.hex --reset

         * If you reflashed the :file:`nordic_aws_nrf52840.hex` file on an already working device, you need to deregister the previously flashed device.
           To do this, perform a factory reset by long pressing **Button 1**.
           This will allow you to register a new product (new :file:`nordic_aws_nrf52840.hex`) in the Sidewalk network.

   .. group-tab:: nRF54 DKs

      1. Follow the `Provision your Sidewalk endpoint and flash the binary image`_ documentation.

         The default address of the :file:`mfg.hex` file provided in the official Amazon Sidewalk documentation is incompatible with the nRF Connect SDK applications.
         To fix it, you must add an argument to the :file:`provision.py` script in order to generate the :file:`mfg.hex` file that is compatible with the nRF Connect SDK memory map.
         Depending on the device, the argument will differ.

         * If you are using the combined device JSON file obtained from the AWS IoT console, use the ``certificate_json`` parameter.
           It will specify this file as an input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin --certificate_json certificate.json --addr 0x17c000

         * If you are using separate device JSON files obtained as responses from the GetDeviceProfile and GetWirelessDevice API operations, use the ``wireless_device_json`` and ``device_profile_json`` parameters.
           This will specify both files as input when running the provisioning script.

            .. parsed-literal::
               :class: highlight

               python3 provision.py nordic aws --output_bin mfg.bin
               --wireless_device_json wireless_device.json
               --device_profile_json device_profile.json
               --addr 0x17c000
               --output_hex nordic_aws_nrf54l15.hex

            .. note::

               For the nRF54L10 SoC emulating on the nRF54L15 DK, you must set the address value to ``--addr 0xFF000``.

            .. note::
               You can use the ``--output_hex`` parameter to specify a custom name for the :file:`mfg.hex` file.
               This change is optional and does not affect the file's compatibility with other supported boards.
               If the parameter is not provided, the default name will be :file:`nordic_aws_nrf52840.hex`.

      #. Flash the generated file with the provisioning data:

         .. code-block:: console

            nrfutil device program --x-family nrf54l --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ --traits jlink --firmware nordic_aws_nrf54l15.hex

         * If you reflashed the :file:`nordic_aws_nrf54l15.hex` file on an already working device, you need to deregister the previously flashed device.
           To do this, perform a factory reset by long pressing **Button 0**.
           This will allow you to register a new product (new :file:`nordic_aws_nrf54l15.hex`) in the Sidewalk network.

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

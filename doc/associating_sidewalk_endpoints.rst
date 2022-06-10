.. _reg_associating_sidewalk_endpoints:

Registering and associating your Sidewalk Endpoints
###################################################

This section outlines how to register your Sidewalk Endpoint and associate it with your Amazon account before it can be connected to Sidewalk.

.. _reg_associating_creating_LwA:

Creating Login with Amazon (LwA) security profile
*************************************************

#. Connect to `Dashboard`_ and log in with your Amazon credentials.
   Credentials are the same as the ones you used to provision your Sidewalk device certificates within the Sidewalk console in :ref:`endpoints_config_and_provisioning`.

#. Go to the :guilabel:`Login with Amazon` tab.

   .. figure:: images/Step6-LoginWithAmazon.jpg

#. Click :guilabel:`Create a New Security Profile`.

   .. figure:: images/Step6-CreateSecurityProfile.jpg

#. Populate the required fields and once completed click :guilabel:`Save`.
   Ensure to use the link to your own Data Privacy policy.

   .. figure:: images/Step6-CreateSecutityProfile-Fill.jpg

#. Save the changes.
   Your new security profile will appear on the list.

   .. figure:: images/Step6-SecurityProfilesList.jpg

#. Click the sprocket wheel icon to go to Web Settings configuration.

#. Copy-paste your :guilabel:`Client ID` from the Web Settings tab to a text editor for easy access.
   These values are required to complete Sidewalk Endpoint registration.

   .. figure:: images/Step6-ClientID.jpg

#. Click :guilabel:`Edit`.

#. In :guilabel:`Allowed Origins` box enter ``http://localhost:8000/``.

#. Click :guilabel:`Save`.

   .. figure:: images/Step6-Localhost.jpg

.. _reg_associating_reg_script:

Using Sidewalk Endpoint Registration Script
*******************************************

Once you have Login with Amazon (LwA) security profile, client-id and client-secret, you can complete the registration of your Sidewalk Endpoint.
You will need a Bluetooth adapter (for example, `Kinivo USB Bluetooth Adapter`_) on your Ubuntu machine to communicate with your Sidewalk Endpoint.

.. note::
    Before proceeding ensure that you have installed all the required Bluetooth adapter drivers.

Configuring your client information
===================================

#. Navigate to :file:`sid_pc_link/apps/device_registration/`.

   .. code-block:: console

       $ cd ${NCS_FOR_SIDEWALK_SDK_PATH}/tools/sid_pc_link/apps/device_registration

#. Install the requirements.

   .. code-block:: console

       pip3 install --user -r requirements.txt

#. Fetch the LwA token only.

   .. code-block:: console

       python3 main.py --lwa --client-id [your client ID]

   This option will launch a Web browser and request your Amazon Developer account credentials.
   Ensure to allow pop-ups.
   LwA token will be auto-populated in your :file:`app_config.json` file.

   a. In case you need to check your client ID, go to `Amazon Developer portal`_.
   #. Navigate to :file:`Security Profile → Web Settings → Client ID` to copy your client ID.

#. Fetch the LwA token and refresh it.

   .. code-block:: console

       python3 main.py --lwa-cg --client-id [your client ID] --client-secret [your client secret]

   This option will launch a Web browser and request your Amazon Developer account credentials.
   Ensure to allow pop-ups.
   LwA and refresh token will be auto-populated in your :file:`app_config.json` file.

#. Refresh your LwA token.

   .. code-block:: console

       python3 main.py --refresh-token --client-id [your client ID] --client-secret [your client secret]

   This option will automatically refresh your LwA token. It uses the previously generated refresh token.
   LwA token will be auto-populated in your :file:`app_config.json` file.

   .. note::
       Ensure to successfully execute LwA token at least once.

   .. note::
       LwA tokens are valid for 1 hour and have to be refreshed after they expire.
       Refresh tokens are valid forever.

Configuring the registration script
===================================

#. Set :guilabel:`BLUETOOTH_ADAPTER` in the :file:`app_config.json` file.
   Run hcitool devices to get the adapter information.

   .. code-block:: console

      $ hcitool devices
      Devices:
         hci1	01:02:03:04:05:06
         hci0	AA:BB:CC:DD:EE:FF

#. Set :guilabel:`ENDPOINT_ID` in the :file:`app_config.json` file.
   This is your device's :guilabel:`SMSN` contained within the device's certificate JSON generated from Sidewalk console (:file:`certificate_${SIDEWALK_ID}.json` file).

   .. code-block:: console

      # app_config.json
      {
          "REGISTRATION_ENVIRONMENT": "prod",
          "BLUETOOTH_ADAPTER": "hci0",
          "COMMAND_TIMEOUT": "20",
          "GATEWAY_ID": null,
          "ENDPOINT_ID": "60BD4AA3FB5B2891813FF736516ACBB76277778B9DE5D06710A76277EXAMPLE",
          "AUTH_TOKEN": null,
          "LWA_TOKEN": "Bearer Atza|...",
          "REFRESH_TOKEN": null,
          "SCAN_RETRIES": "5",
          "BLUETOOTH_SCAN_TIMEOUT": "5"
      }

#. Run Sidewalk Endpoint registration script.

   .. code-block:: console

       $ python3 main.py -r

   The script will return the following output:

   .. code-block:: console

        apps.device_registration.device_registration[10309] INFO Device registration succeeded


   Once you have completed the registration of your Sidewalk Endpoint, it is now ready to send and receive data over Sidewalk.

.. note::

    Before going to next steps, ensure you have the following:

       * A Sidewalk Endpoint registered using the :file:`sid_pc_link` script.
         The script emulates the device registration process performed by the Sidewalk Mobile SDK. I
         t sends commands to the device as it communicates with the Sidewalk Cloud and your application server.

       * A Login With Amazon (LwA) security profile created.
         The LwA security profile and :file:`sid_pc_link` LwA help files emulate the registration flow, in which the end-user needs to log in with their Amazon account and grant Sidewalk permissions to complete Sidewalk registration for the device.

.. _Dashboard: https://developer.amazon.com/dashboard
.. _Kinivo USB Bluetooth Adapter: https://www.amazon.com/Kinivo-USB-Bluetooth-4-0-Compatible/dp/B007Q45EF4
.. _Amazon Developer Portal: https://developer.amazon.com/

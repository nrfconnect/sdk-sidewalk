.. _reg_associating_sidewalk_endpoints:

Manual registration using local machine and Registration Script
###############################################################

This section outlines how to register your Sidewalk Endpoint and associate it with your Amazon account before it can be connected to Sidewalk.

.. note::
   To be able to perform manual registration steps, you need to have the developer account and you need to be allow-listed by Amazon.

.. _reg_associating_creating_LwA:

Creating Login with Amazon (LwA) security profile
*************************************************

#. Connect to `Dashboard`_ and log in with your Amazon credentials.

#. Go to the :guilabel:`Login with Amazon` tab.

   .. figure:: /images/Step6-LoginWithAmazon.jpg

#. Click :guilabel:`Create a New Security Profile`.

   .. figure:: /images/Step6-CreateSecurityProfile.jpg

#. Populate the required fields. Once completed, click :guilabel:`Save`.
   Ensure you use the link to your own Data Privacy policy.

   .. figure:: /images/Step6-CreateSecutityProfile-Fill.jpg

#. Save the changes.
   Your new security profile will appear on the list.

   .. figure:: /images/Step6-SecurityProfilesList.jpg

#. Click the sprocket wheel icon to go to the :guilabel:`Web Settings` configuration.

#. Copy-paste your :guilabel:`Client ID` from the :guilabel:`Web Settings` tab to a text editor for an easy access.
   These values are required to complete the Sidewalk Endpoint registration.

   .. figure:: /images/Step6-ClientID.jpg

#. Click :guilabel:`Edit`.

#. In :guilabel:`Allowed Origins` box, enter ``http://localhost:8000/``, and click :guilabel:`Save`.

   .. figure:: /images/Step6-Localhost.jpg

.. _reg_associating_reg_script:

Using Sidewalk Endpoint Registration Script
*******************************************

Once you have the Login with Amazon (LwA) security profile, client-id and client-secret, you can complete the registration of your Sidewalk Endpoint.
You will need a Bluetooth adapter (for example, `Kinivo USB Bluetooth Adapter`_) on your machine to communicate with your Sidewalk Endpoint.

.. note::
    Before proceeding, ensure that you have installed all the required Bluetooth adapter drivers.

Configuring your client information
===================================

#. Navigate to the :file:`device_registration` folder:

   .. code-block:: console

       $ cd ${ZEPHYR_BASE}/../sidewalk/tools/sid_pc_link/apps/device_registration

#. Install Python requirements.

   .. code-block:: console

      $ pip install -r requirements.txt

#. Fetch the LwA token only.

   .. code-block:: console

       python3 main.py --lwa --client-id [your client ID]

   This option will launch a web browser and request your Amazon Developer account credentials.
   Ensure to allow pop-ups.
   LwA token will be auto-populated in the :file:`app_config.json` file.

   a. In case you need to check your client ID, go to `Amazon Developer portal`_.
   #. Navigate to :file:`Security Profile → Web Settings → Client ID`, and copy your client ID.

#. Fetch the LwA token, and refresh it.

   .. code-block:: console

       python3 main.py --lwa-cg --client-id [your client ID] --client-secret [your client secret]

   This option will launch a web browser and request your Amazon Developer account credentials.
   Ensure to allow pop-ups.
   LwA and refresh token will be auto-populated in the :file:`app_config.json` file.

#. Refresh your LwA token.

   .. code-block:: console

       python3 main.py --refresh-token --client-id [your client ID] --client-secret [your client secret]

   This option will automatically refresh your LwA token.
   It uses the previously generated refresh token.
   LwA token will be auto-populated in the :file:`app_config.json` file.

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
   This is the :guilabel:`SMSN` of your device contained within the device's certificate that JSON generated from the Sidewalk console (:file:`certificate_${SIDEWALK_ID}.json` file):

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

#. Run the Sidewalk Endpoint registration script.

   .. code-block:: console

       $ python3 main.py -r

   The script will return the following output:

   .. code-block:: console

        apps.device_registration.device_registration[10309] INFO Device registration succeeded

   Once you have completed the registration of your Sidewalk Endpoint, it is now ready to send and receive data over Sidewalk.

Expected results
****************

Before going to next steps, ensure you have the following:

   * A Sidewalk Endpoint registered using the :file:`sid_pc_link` script.
     The script emulates the device registration process performed by the Sidewalk Mobile SDK.
     It sends commands to the device as it communicates with the Sidewalk Cloud and your application server.

   * A Login With Amazon (LwA) security profile created.
     The LwA security profile and :file:`sid_pc_link` LwA help files emulate the registration flow, in which the end user needs to log in with their Amazon account and grant Sidewalk permissions to complete Sidewalk registration for the device.

.. _Dashboard: https://developer.amazon.com/dashboard
.. _Kinivo USB Bluetooth Adapter: https://www.amazon.com/Kinivo-USB-Bluetooth-4-0-Compatible/dp/B007Q45EF4
.. _Amazon Developer Portal: https://developer.amazon.com/

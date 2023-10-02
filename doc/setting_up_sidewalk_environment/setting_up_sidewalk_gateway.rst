.. _setting_up_sidewalk_gateway:

Setting up Sidewalk Gateway
###########################

You must set up and enable Sidewalk on the latest generation `Echo`_ device.
The device is used as a Sidewalk Gateway and you can configure it with the Alexa app (`iOS`_ or `Android`_).
Once your Sidewalk Endpoint is registered, it will connect to the Sidewalk Gateway and communicate with it.

.. note::
   Refer to the `Amazon Sidewalk`_ for the complete list of available Sidewalk Gateways.

.. _setting_up_sidewalk_gateway_exising_device:

Enabling Ring device as a Sidewalk Gateway
******************************************

If you are setting up a new Echo device, complete the setup through the Alexa app.
For an existing Echo device, go to the :ref:`setting_up_sidewalk_gateway_new_device` section.

#. Download the `Alexa app`_ and open it.
#. Click :guilabel:`More`, and select :guilabel:`Add device`.
#. Plug in your device.
#. Follow the instructions in your mobile app.
#. Click :guilabel:`Enable` in the landing page.

   .. figure:: /images/sidewalk_alexa_image1.png

.. _setting_up_sidewalk_gateway_new_device:

Enabling Sidewalk on an existing Echo device
********************************************

If you already have an existing Echo device, check the device settings to determine whether your device can act as a Sidewalk Gateway.

#. Open :guilabel:`More`, and select :guilabel:`Settings`.
#. Click :guilabel:`Account Settings`.
#. Select :guilabel:`Amazon Sidewalk` and enable it.

   .. figure:: /images/sidewalk_alexa_image2.png

.. _setting_up_sidewalk_gateway_linking_ring_amazon_account:

Linking your Ring account to your Amazon account
************************************************

You can link your Ring account to your Amazon account using the Ring app.

#. Open the Ring page in `App store`_ (iOS) or `Play store`_ (Android).
#. Download and install the Ring app on your mobile device.
#. Create a new account, or log into your existing Ring account.
#. Open the Ring application.
#. Log into your account.
   If you do not have a Ring account, create it:

   a. Select :guilabel:`Create Account`.
   #. Follow the instructions provided in the application.
   #. Click :guilabel:`Enable` in the landing page.

   .. figure:: /images/sidewalk_ring_image1.png

#. Click the menu button, and navigate to :guilabel:`Account Settings`.
#. Click :guilabel:`Link your Amazon account` and then :guilabel:`Link Accounts`.
#. Enter the password for your Ring account.
#. Log into your Amazon Account you used to provision your Sidewalk Endpoint in the Sidewalk console.
#. Click :guilabel:`Sign in`.
   Your accounts are now linked.

   .. figure:: /images/sidewalk_ring_image2.png

.. note::
   If you have multiple Echo devices registered under a single Amazon account, only one of those devices will be able to operate as an FSK gateway even if other devices have the same capability.
   If you need to operate multiple FSK gateways, we recommend setting them up under separate Amazon accounts.

.. _Echo: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id94401162
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app
.. _Amazon Sidewalk: https://www.amazon.com/sidewalk
.. _Alexa app: https://www.amazon.com/b?node=18354642011
.. _App store: https://ring.com/ios
.. _Play store: https://ring.com/android

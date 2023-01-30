.. _sidewalk_requirements:

Requirements
************

Ensure the requirements below are met:

#. You are located in the US.

   .. note::
       If you set up a non-US location during the account creation, Sidewalk might not be available.

#. You created the following accounts:

   - An `Amazon developer account`_ to be able to access Sidewalk console and Sidewalk technical documentation.

   - An `Amazon Web Service account`_ to be able to connect AWS cloud applications with your Sidewalk Endpoints.

#. You have `Sidewalk console`_.
   It is used to define a new Sidewalk-enabled product.
#. You installed `AWS CLI`_ version 2 configuration to send data to your Sidewalk Endpoints.
#. You have native Ubuntu machine running (version 18.04 or higher) for device registration.

   .. note::
      * Operations in this quick start guide are based on common Linux commands and software packages.
      * Python3 and python3-pip are required to run Sidewalk tools.
      * Using virtual machine is not recommended due to known compatibility issues.

#. You have a hardware development kit:

   - `Nordic nRF52840-DK`_ - nRF52840 MCU development kit.
   - `Semtech SX1262 mbed shield eval board`_ - 900 MHz radio shield.
   - `Nordic nRF52840-DK`_ - MX25R6435F external flash on Nordic DK or similar for dual bank firmware update.

   .. note::
      To test Sidewalk over Bluetooth LE you only need Nordic nRF52840-DK.

#. You have a Bluetooth wireless interface on your Ubuntu machine or USB Bluetooth adapter.

   Bluetooth wireless adapter is used during device registration to connect the Ubuntu machine with the hardware development kit.
   For example `USB Bluetooth adapter`_, recommended by Amazon.

#. You have a Sidewalk Gateway.

   `Echo`_ is used as the Sidewalk Gateway.
   It is configured with the Alexa App on `iOS`_ or `Android`_ and requires an Amazon account for setup.


.. _Amazon developer account: https://developer.amazon.com/dashboard
.. _Amazon Web Service account: https://console.aws.amazon.com/console/home
.. _Sidewalk console: https://developer.amazon.com/acs-devices/console/sidewalk
.. _AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-install.html
.. _Nordic nRF52840-DK: https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262 mbed shield eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _USB Bluetooth adapter: https://www.amazon.com/Kinivo-USB-Bluetooth-4-0-Compatible/dp/B007Q45EF4
.. _Echo: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id944011620
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app
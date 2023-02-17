.. _sidewalk_requirements:

Requirements
************

 .. note::
    Sidewalk is currently available only in the USA.

Ensure the requirements below are met:

#. You created the following accounts:

   - An `Amazon Web Service account`_ to be able to connect AWS cloud applications with your Sidewalk Endpoints.

#. You have installed `AWS CLI`_ with Sidewalk commends support in order to send data to your Sidewalk Endpoints.

#. You have native Ubuntu machine running (version 18.04 or higher) for device registration.

   .. note::
      * Operations in this quick start guide are based on common Linux commands and software packages.
      * Python3 and python3-pip are required to run Sidewalk tools.

#. You have a hardware development kit:

   - `Nordic nRF52840-DK`_ - nRF52840 MCU development kit.
   - `Semtech SX1262 mbed shield eval board`_ - sub-GHz radio shield.
   - `Nordic nRF52840-DK`_ - MX25R6435F external flash on Nordic DK or similar for dual bank firmware update.

   .. note::
      To test Sidewalk over Bluetooth LE you only need Nordic nRF52840-DK.

#. You have a Bluetooth wireless interface on your Ubuntu machine or USB Bluetooth adapter.

   Bluetooth wireless adapter is used during device registration to connect the Ubuntu machine with the hardware development kit.
   For example `USB Bluetooth adapter`_, recommended by Amazon.

#. You have a Sidewalk Gateway.

   `Echo 4th Gen`_ is used as the Sidewalk Gateway.
   It is configured with the Alexa App on `iOS`_ or `Android`_ and requires an Amazon account for setup.

.. _requirements_memory:

RAM and flash memory requirements
*********************************

RAM and flash memory requirement values differ depending on the DK and the programmed sample.

The following tables list memory requirement values for the Sidewalk samples.

Values are provided in kilobytes (KB).

.. tabs::

   .. tab:: nRF52840 DK

      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | Sample                                                       |   MCUBoot ROM |   Application ROM |   Settings |   Total ROM |   Total RAM |
      +==============================================================+===============+===================+============+=============+=============+
      | Sensor monitoring (Debug)                                    |             0 |               371 |          0 |         371 |          74 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` (Debug)                    |            28 |               379 |          8 |         415 |          74 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` (Release)                  |            28 |               317 |          8 |         353 |          70 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` FSK (Debug)                |            28 |               477 |          8 |         513 |          91 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` FSK (Release)              |            28 |               409 |          8 |         445 |          86 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` LoRa (Debug)               |            28 |               477 |          8 |         513 |          91 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template <template_sample>` LoRa (Release)             |            28 |               323 |          8 |         359 |          73 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble_sample>` (Debug)   |            28 |               379 |          8 |         415 |          74 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble_sample>` (Release) |            28 |               317 |          8 |         353 |          70 |
      +--------------------------------------------------------------+---------------+-------------------+------------+-------------+-------------+

.. _Amazon developer account: https://developer.amazon.com/dashboard
.. _Amazon Web Service account: https://console.aws.amazon.com/console/home
.. _AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-install.html
.. _Nordic nRF52840-DK: https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262 mbed shield eval board: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _USB Bluetooth adapter: https://www.amazon.com/Kinivo-USB-Bluetooth-4-0-Compatible/dp/B007Q45EF4
.. _Echo 4th Gen: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id944011620
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app
.. _nrf52840 DK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.1.0-rc2/nrf/app_boards.html#board-names

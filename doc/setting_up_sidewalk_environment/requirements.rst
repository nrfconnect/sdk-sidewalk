.. _sidewalk_requirements:

Requirements
############

This page outlines the requirements that you need to meet before you start working with the Amazon Sidewalk environment.

.. note::
   Amazon Sidewalk is available only in the United States of America.
   To an extent, any Sidewalk gateway functionality might be used outside of the U.S., however, it should be used only for the Amazon Sidewalk endpoint development purposes.
   In addition, we recommend that you consult with your local regulatory bodies, and check if the gateway is allowed to operate its radio in your locale, as U.S. license-free band devices, only for development.

Software requirements
*********************

To meet the software requirements, complete the following:

* Create an `Amazon Web Service account`_ to be able to connect AWS cloud applications with your Sidewalk Endpoints.

* Install `AWS CLI`_ with the Sidewalk command support to be able to send data to your Sidewalk Endpoints.

.. note::
   * Python3 and python3-pip are required to run the Sidewalk tools.
   * This solution was tested on Ubuntu Linux 20.04 LTS and 22.04 LTS.

.. _hardware_requirements:

Hardware requirements
*********************

Ensure you have the following:

* A supported Nordic Semiconductor's development kit and, if required, other additional hardware.
  For the list of the required devices per sample, see :ref:`samples_list`.

* Bluetooth wireless interface on your machine or a USB Bluetooth adapter.

   .. note::
      Bluetooth wireless adapter is used during the device registration to connect the machine with the development kit.
      For example, you can use a `USB Bluetooth adapter`_ recommended by Amazon.

* Sidewalk Gateway

   You can use one of the supported `Amazon Sidewalk Gateways`_, for example, `Echo 4th Gen`_.
   It is configured with the Alexa App on `iOS`_ or `Android`_ and requires an Amazon account for setup.

.. _requirements_memory:

RAM and flash memory requirements
*********************************

RAM and flash memory requirement values differ depending on the DK and programmed sample.

All values are provided in kilobytes (KB).

.. tabs::

   .. tab:: nRF52840 DK

      The following table lists memory requirements for samples running on the `nRF52840 DK`_.

      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | Sample                                                            |   MCUboot ROM |   Application ROM |   Sidewalk Settings |   Total ROM |   Total RAM |
      +===================================================================+===============+===================+=====================+=============+=============+
      | :ref:`Sensor monitoring <sensor_monitoring>` (Bluetooth LE Debug) |             0 |               371 |                  32 |         403 |          78 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (FSK Debug)          |             0 |               530 |                  32 |         562 |          99 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (LoRa Debug)         |             0 |               470 |                  32 |         502 |          98 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (FSK Debug)             |            36 |               547 |                  28 |         611 |         109 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (FSK Release)           |            36 |               479 |                  28 |         543 |         105 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (LoRa Debug)            |            36 |               488 |                  28 |         552 |         108 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (LoRa Release)          |            36 |               413 |                  28 |         477 |         104 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble>` (Debug)               |            36 |               384 |                  28 |         448 |          90 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble>` (Release)             |            36 |               314 |                  28 |         378 |          86 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+

   .. tab:: nRF5340 DK

      The following table lists memory requirements for samples running on the `nRF5340 DK`_.

      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | Sample                                                            |   MCUboot ROM |   Application ROM |   Sidewalk Settings |   Total ROM |   Total RAM |
      +===================================================================+===============+===================+=====================+=============+=============+
      | :ref:`Sensor monitoring <sensor_monitoring>` (Bluetooth LE Debug) |             0 |               314 |                  32 |         346 |          68 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (FSK Debug)          |             0 |               463 |                  32 |         495 |          89 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (LoRa Debug)         |             0 |               411 |                  32 |         443 |          88 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (FSK Debug)             |            40 |               479 |                  28 |         547 |          99 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (FSK Release)           |            40 |               410 |                  28 |         478 |          95 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (LoRa Debug)            |            40 |               428 |                  28 |         496 |          98 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template sub-GHz <template_subghz>` (LoRa Release)          |            40 |               352 |                  28 |         420 |          94 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble>` (Debug)               |            40 |               333 |                  28 |         401 |          81 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Template Bluetooth LE <template_ble>` (Release)             |            40 |               260 |                  28 |         328 |          77 |
      +-------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
.. include:: ../ncs_links.rst

.. _Amazon developer account: https://developer.amazon.com/dashboard
.. _Amazon Web Service account: https://console.aws.amazon.com/console/home
.. _AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-install.html
.. _Nordic nRF52840 DK: https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _USB Bluetooth adapter: https://www.amazon.com/Kinivo-USB-Bluetooth-4-0-Compatible/dp/B007Q45EF4
.. _Echo 4th Gen: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _Amazon Sidewalk Gateways: https://docs.sidewalk.amazon/introduction/sidewalk-gateways.html
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id944011620
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app

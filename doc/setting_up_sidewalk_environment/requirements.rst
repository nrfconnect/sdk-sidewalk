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

      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | Sample                                                              |   MCUboot ROM |   Application ROM |   Sidewalk Settings |   Total ROM |   Total RAM |
      +=====================================================================+===============+===================+=====================+=============+=============+
      | :ref:`Sensor monitoring <sensor_monitoring>` (Bluetooth LE Debug)   |             0 |               364 |                  32 |         396 |          78 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (FSK Debug)            |             0 |               517 |                  32 |         549 |          98 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (LoRa Debug)           |             0 |               464 |                  32 |         496 |          97 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Debug)                |            36 |               478 |                  28 |         542 |         109 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Bluetooth LE Debug)   |            36 |               378 |                  28 |         442 |          91 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Bluetooth LE Release) |            36 |               313 |                  28 |         377 |          86 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Release)              |            36 |               407 |                  28 |         471 |         104 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+

   .. tab:: nRF5340 DK

      The following table lists memory requirements for samples running on the `nRF5340 DK`_.

      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | Sample                                                              |   MCUboot ROM |   Application ROM |   Sidewalk Settings |   Total ROM |   Total RAM |
      +=====================================================================+===============+===================+=====================+=============+=============+
      | :ref:`Sensor monitoring <sensor_monitoring>` (Bluetooth LE Debug)   |             0 |               309 |                  32 |         341 |          67 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (FSK Debug)            |             0 |               449 |                  32 |         481 |          87 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sensor monitoring <sensor_monitoring>` (LoRa Debug)           |             0 |               405 |                  32 |         437 |          86 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Debug)                |            40 |               419 |                  28 |         487 |          98 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Bluetooth LE Debug)   |            40 |               324 |                  28 |         392 |          81 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Bluetooth LE Release) |            40 |               255 |                  28 |         323 |          76 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+
      | :ref:`Sidewalk template <sidewalk_template>` (Release)              |            40 |               346 |                  28 |         414 |          93 |
      +---------------------------------------------------------------------+---------------+-------------------+---------------------+-------------+-------------+

.. include:: ../ncs_links.rst

.. _Amazon developer account: https://developer.amazon.com/dashboard
.. _Amazon Web Service account: https://console.aws.amazon.com/console/home
.. _AWS CLI: https://docs.aws.amazon.com/cli/latest/userguide/cli-chap-install.html
.. _Nordic nRF52840 DK: https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-DK
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
.. _Echo 4th Gen: https://www.amazon.com/All-New-Echo-4th-Gen/dp/B07XKF5RM3
.. _Amazon Sidewalk Gateways: https://docs.sidewalk.amazon/introduction/sidewalk-gateways.html
.. _iOS: https://apps.apple.com/us/app/amazon-alexa/id944011620
.. _Android: https://play.google.com/store/apps/details?id=com.amazon.dee.app

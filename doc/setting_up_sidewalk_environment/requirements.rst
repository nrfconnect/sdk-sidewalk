.. _sidewalk_requirements:

Requirements
############

.. contents::
   :local:
   :depth: 2

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

* Sidewalk Gateway.
  You can use one of the supported `Amazon Sidewalk Gateways`_, for example, `Echo 4th Gen`_.
  It is configured with the Alexa App on `iOS`_ or `Android`_ and requires an Amazon account for setup.

.. _requirements_memory:

RAM and flash memory requirements
*********************************

RAM and flash memory requirement values differ depending on the DK and programmed sample.


.. tabs::

   .. tab:: nRF52840 DK

      The following table lists memory requirements (without MCUboot partition) for the default variant of the :ref:`Sidewalk_End_device` sample running on the `nRF52840 DK`_.

      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      | Sample                                                                                                         |   App partition size |   App RAM |   Total App partition size |   Total RAM |
      +================================================================================================================+======================+===========+============================+=============+
      |   :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Debug)    |      412.20 kB       |  94.08 kB |         983.50 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |  :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Release)   |      351.50 kB       |  89.21 kB |         983.50 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |                :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Debug)                 |      494.87 kB       | 109.58 kB |         983.50 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |               :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Release)                |      430.05 kB       | 104.71 kB |         983.50 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+

   .. tab:: nRF5340 DK

      The following table lists memory requirements (without MCUboot partition) for the default variant of the :ref:`Sidewalk_End_device` sample running on the `nRF5340 DK`_.

      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      | Sample                                                                                                         |   App partition size |   App RAM |   Total App partition size |   Total RAM |
      +================================================================================================================+======================+===========+============================+=============+
      |   :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Debug)    |      354.09 kB       |  82.97 kB |         975.50 kB          |  440.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |  :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Release)   |      290.48 kB       |  78.26 kB |         975.50 kB          |  440.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |                :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Debug)                 |      432.80 kB       |  98.18 kB |         975.50 kB          |  440.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |               :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Release)                |      365.91 kB       |  93.46 kB |         975.50 kB          |  440.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+

   .. tab:: nRF54L15 PDK

      The following table lists memory requirements (without MCUboot partition) for the default variant of the :ref:`Sidewalk_End_device` sample running on the `nRF54L15 PDK`_.

      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      | Sample                                                                                                         |   App partition size |   App RAM |   Total App partition size |   Total RAM |
      +================================================================================================================+======================+===========+============================+=============+
      |   :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Debug)    |      350.58 kB       |  87.74 kB |         730.00 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |  :ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>` (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Release)   |      285.88 kB       |  82.89 kB |         730.00 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |                :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Debug)                 |      350.83 kB       |  87.74 kB |         730.00 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+
      |               :ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>` (Release)                |      286.00 kB       |  82.89 kB |         730.00 kB          |  256.00 kB  |
      +----------------------------------------------------------------------------------------------------------------+----------------------+-----------+----------------------------+-------------+

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

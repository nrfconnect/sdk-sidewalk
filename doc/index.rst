.. _index:

.. caution::
   Sidewalk protocol is owned and licensed by Amazon.
   Before continuing with the nRF Connect SDK for Sidewalk, you must familiarize yourself with the official `Amazon license`_, including all the applicable restrictions.
   Please note, that redistributing Sidewalk under the nRF Connect SDK is prohibited.

   Be aware, that by downloading and including the Sidewalk repository, you accept the above-mentioned license.

   Due to these restrictions, Sidewalk cannot be automatically fetched through the nRF Connect SDK and will require performing a dedicated setup.
   All information regarding completing it is included in this guide, so ensure you follow it in order.

Welcome to the nRF Connect SDK - Amazon Sidewalk
################################################

Amazon Sidewalk is a shared network designed to provide a stable and reliable connection to your devices allowing them to work better at your home and outside of it.
Ring and Echo device can act as a gateway, meaning they can share a portion of internet bandwidth providing the connection and services to Sidewalk end devices.
For the list of supported devices, see `Amazon Sidewalk Gateways`_.

.. note::
   Amazon Sidewalk is available only in the United States of America.
   To an extent, any Sidewalk gateway functionality might be used outside of the U.S., however, it should be used only for the Amazon Sidewalk endpoint development purposes.
   In addition, we recommend that you consult with your local regulatory bodies, and check if the gateway is allowed to operate its radio in your locale, as U.S. license-free band devices, only for development.

Amazon Sidewalk for the nRF Connect SDK is based on two variants, one using Bluetooth® LE (more suited for home applications) and the other one using sub-GHz with the Semtech radio transceiver (for applications requiring longer range).
The latter also expands to two options: Sidewalk over LoRa and Sidewalk over FSK (having a shorter range but with higher throughput compared to LoRa).

Amazon Sidewalk products can be tested and developed using the Nordic Semiconductor's development kit together with the `nRF Connect SDK`_.
For more details on the supported devices, see the :ref:`hardware_requirements` section.
The software development kit allows to build software that is optimized for size and performance, especially when working with devices that have limited memory.
The SDK integrates the latest version of the Amazon Sidewalk repository.

To learn more about the Amazon Sidewalk solution, refer to the :ref:`additional_resources` page.

This documentation will guide you through the setup of Amazon Sidewalk used with the Nordic Semiconductor's technology.
Browse the content by referring to the following pages:

.. toctree::
   :maxdepth: 2
   :glob:
   :caption: Subpages:

   setting_up_sidewalk_environment/setting_up_environment.rst
   samples/samples_list.rst
   compatibility_matrix.rst
   releases_and_migration.rst
   additional_resources.rst

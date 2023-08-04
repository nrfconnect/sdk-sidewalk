.. _samples_list:

Samples overview
################

.. note::
    For the best user experience, start your work with the Sensor Monitoring application.

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The following table demonstrates configuration for the supported development kits and differences between samples:

.. tabs::

   .. tab:: nRF52840

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader                                    | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sensor_monitoring`    | * `Semtech SX1262MB2CAS`_ (optional)                                     | --                                            | * :file:`fsk.conf`            | Supported             | Supported    | Supported   | --          |
      |                             |                                                                          |                                               | * :file:`lora.conf`           |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`template_subghz`      | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | * :file:`fsk.conf`            | ``**``                | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`template_ble`         | --                                                                       | * MCUboot - DFU partition in internal flash   | --                            | Supported             | --           | --          | Supported   |
      |                             |                                                                          | * Application partition size = 480 kB         |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`device_under_test`    | * `Semtech SX1262MB2CAS`_ (optional)                                     | --                                            | --                            | Supported             | Supported    | Supported   | --          |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

      ``**`` - Only for DFU and registration in LoRa transport

   .. tab:: nRF5340 ``*``

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader                                    | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sensor_monitoring`    | * `Semtech SX1262MB2CAS`_ (optional)                                     | --                                            | * :file:`fsk.conf`            | Supported             | Supported    | Supported   | --          |
      |                             |                                                                          |                                               | * :file:`lora.conf`           |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`template_subghz`      | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | * :file:`fsk.conf`            | ``**``                | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`template_ble`         | --                                                                       | * MCUboot - DFU partition in internal flash   | --                            | Supported             | --           | --          | Supported   |
      |                             |                                                                          | * Application partition size = 480 kB         |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | :ref:`device_under_test`    | * `Semtech SX1262MB2CAS`_ (optional)                                     | --                                            | --                            | Supported             | Supported    | Supported   | --          |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

      ``*`` - Experimental support

      ``**`` - Only for DFU and registration in LoRa transport

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: List of samples:

   sidewalk_application_samples/sensor_monitoring.rst
   sidewalk_application_samples/template_subghz.rst
   sidewalk_application_samples/template_ble.rst
   sidewalk_application_samples/dut.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

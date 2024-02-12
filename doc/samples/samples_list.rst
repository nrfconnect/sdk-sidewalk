.. _samples_list:

Samples overview
################

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The following table demonstrates configuration for the supported development kits and differences between samples:

.. tabs::

   .. tab:: nRF52840

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader (in release and debug configs)     | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sidewalk_template`    | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | * ``overlay-dut.conf``        | Supported             | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

   .. tab:: nRF5340

      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
      | Sample                      | Additional hardware                                                      | Bootloader (in release and debug configs)     | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
      +=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
      | :ref:`sidewalk_template`    | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | * ``overlay-dut.conf``        | Supported             | Supported    | Supported   | Supported   |
      |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         |                               |                       |              |             |             |
      |                             |                                                                          |                                               |                               |                       |              |             |             |
      +-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: New samples migration guide:

   why_new_template.rst

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: List of samples:

   template.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

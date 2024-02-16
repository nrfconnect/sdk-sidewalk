.. _samples_list:

Samples overview
################

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.

The following table demonstrates configuration for the supported development kits:

+-----------------------------+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+
| Development kit             | Sample                      | Additional hardware                                                      | Bootloader (in release and debug configs)     | Application overlays          | Bluetooth® LE support | LoRa support | FSK support | DFU support |
+=============================+=============================+==========================================================================+===============================================+===============================+=======================+==============+=============+=============+
|  * `nRF52840 DK`_           | :ref:`sidewalk_template`    | * External flash (included in the DK)                                    | * MCUboot - DFU partition in external flash   | * ``overlay-dut.conf``        | Supported             | Supported    | Supported   | Supported   |
|  * `nRF5340 DK`_            |                             | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) | * Application partition size = 956 kB         | * ``overlay-demo.conf``       |                       |              |             |             |
|                             |                             |                                                                          |                                               | * ``overlay-hello.conf``      |                       |              |             |             |
+-----------------------------+-----------------------------+--------------------------------------------------------------------------+-----------------------------------------------+-------------------------------+-----------------------+--------------+-------------+-------------+


.. note::

   With the v2.6.0 release of the `nRF Connect SDK`_ a new sample model has been introduced.
   To see how the changes might affect your application, see the :ref:`migration_guide`.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: List of samples:

   template.rst

.. include:: ../ncs_links.rst

.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas

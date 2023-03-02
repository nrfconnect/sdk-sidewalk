.. _samples_overview:

Samples overview
################

In the Sidewalk repository, all samples are placed in the :file:`samples` directory.
The following table outlines the differences in their configurations:

+-----------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| Sample                | Hardware required                                                        | Bootloader                                    | Application overlays                        | Persistant data size                | Supported transports     |
+=======================+==========================================================================+===============================================+=============================================+=====================================+==========================+
| Template sub-GHz      | * `nRF52840dk_nrf52840`_                                                 | * MCUboot - DFU partition in external flash   | * :file:`fsk.conf`                          | * 8 kB Zephyr settings              | * Bluetooth LE           |
|                       | * External flash (included in the DK)                                    | * Application partition size = 956 kB         | * :file:`ble.conf`                          | * 28 kB Sidewalk key-value storage  | * LoRa                   |
|                       | * `Semtech SX1262MB2CAS`_ (only for the configurations with LoRa or FSK) |                                               | * :file:`usb_dfu`                           |                                     | * FSK                    |
+-----------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| Template Bluetooth LE | * `nRF52840dk_nrf52840`_                                                 | * MCUboot - DFU partition in internal flash   | * :file:`usb_dfu`                           | * 8kB Zephyr settings               | * Bluetooth LE           |
|                       |                                                                          | * Application partition size = 480 kB         |                                             | * 24 kB Sidewalk key-value storage  |                          |
+-----------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| Sensor Monitoring     | * `nRF52840dk_nrf52840`_                                                 | --                                            | --                                          | * 8 kB Zephyr settings              | * Bluetooth LE           |
|                       | * `Semtech SX1262MB2CAS`_                                                |                                               |                                             | * 24 kB Sidewalk key-value storage  | * LoR                    |
|                       |                                                                          |                                               |                                             |                                     | * FSK                    |
+-----------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+
| Device Under Test     | * `nRF52840dk_nrf52840`_                                                 | --                                            | --                                          | * 8 kB Zephyr settings              | * Bluetooth LE           |
|                       | * `Semtech SX1262MB2CAS`_                                                |                                               |                                             | * 24 kB Sidewalk key-value storage  | * LoRa                   |
|                       |                                                                          |                                               |                                             |                                     | * FSK                    |
+-----------------------+--------------------------------------------------------------------------+-----------------------------------------------+---------------------------------------------+-------------------------------------+--------------------------+


.. _samples_build_type:

Build types
***********

Samples use different configuration files depending on the supported features.
Configuration files are provided for different build types and are located in the application root directory.

The :file:`prj.conf` file represents a ``debug`` build type.
Other build types are covered by dedicated files with the respective build type added to the file name.
For example, the ``release`` build type file is named :file:`prj_release.conf`.
The same naming convention is followed if a board has other configuration files that are, for example, associated with the partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
Depending on the selected board, a sample supports the following build types:

* ``debug`` -- Debug version of the application - can be used to enable additional features, such as logs or command-line shell, to verify the application behavior.
* ``release`` -- Release version of the application - can be used to enable only the necessary application functionalities to optimize its performance.

You can build the ``release`` firmware for ``nrf52840dk_nrf52840`` by running the following command in the project directory:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf

.. note::
    Selecting a build type is optional.
    However, if the build type is not selected, the ``debug`` build type is used by default.

.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.3.0/zephyr/boards/arm/nrf52840dk_nrf52840/doc/index.html
.. _Semtech SX1262MB2CAS: https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262mb2cas
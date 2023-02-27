.. _sidewalk_samples:

Sidewalk application samples
############################

In the Sidewalk repository, samples are placed in the :file:`samples` directory.

To learn more about the samples, refer to the subpages below.
For information about building the sample, see the :ref:`samples_building_and_running` documentation.

.. toctree::
   :maxdepth: 1
   :glob:
   :caption: Subpages:

   sidewalk_application_samples/sensor_monitoring.rst
   sidewalk_application_samples/template_ble.rst
   sidewalk_application_samples/template_subghz.rst
   sidewalk_application_samples/dut.rst

The table below shows differences between their configurations:

+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Criteria              | Template sub-GHz                                              | Template Bluetooth LE                      |
+=======================+===============================================================+============================================+
| Hardware required     | * `nRF52840dk_nrf52840`_                                      | * `nRF52840dk_nrf52840`_                   |
|                       | * External flash (included in the DK)                         |                                            |
|                       | * Semtech sx1262 (only for the configurations with LoRa/FSK)  |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Bootloader            | * MCUBoot - DFU partition in external flash                   | * MCUBoot - DFU partition in internal flash|
|                       | * Application partition size = 956kB                          | * Application partition size = 480kB       |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Application overlays  | * :file:`fsk.conf`                                            | * :file:`usb_dfu`                          |
|                       | * :file:`usb_dfu`                                             |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Persistant data size  | * 8kB Zephyr settings                                         | * 8kB Zephyr settings                      |
|                       | * 28kB Sidewalk key-value storage                             | * 24kB Sidewalk key-value storage          |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+

.. _sidewalk_samples_build_type:

Build type
**********

The sample uses different configuration files depending on the supported features.
Configuration files are provided for different build types and they are located in the application root directory.

The :file:`prj.conf` file represents a ``debug`` build type.
Other build types are covered by dedicated files with the respective build type added to the file name.
For example, the ``release`` build type file is named :file:`prj_release.conf`.
The same naming convention is followed if a board has other configuration files, for example, associated with partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
Depending on the selected board, the sample supports the following build types:

* ``debug`` -- Debug version of the application - can be used to enable additional features, such as logs or command-line shell, to verify the application behavior.
* ``release`` -- Release version of the application - can be used to enable only the necessary application functionalities to optimize its performance.

You can build the ``release`` firmware for ``nrf52840dk_nrf52840`` by running the following command in the project directory:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf
   
.. note::
    Selecting a build type is optional.
    However, if the build type is not selected, the ``debug`` build type is used by default.

.. _sidewalk_samples_ui:

User interface actions
**********************

   * Factory Reset:
      The app informs the Sidewalk stack about the factory reset event.
      The Sidewalk library clears its configuration from the non-volatile storage.
      After a successful reset, the device needs to be registered with the cloud services again.

   * Toggle Connection Request:
      The device requests the Sidewalk gateway to initiate a connection while the device is advertising via BLE.
      After a connection is dropped the user has to set this beacon state again.
      Gateways may not always be able to honour this request.
      It depends on the number of devices connected to it.

   * Send Hello:
      This action will queue a message to the cloud.
      If Sidewalk is not ready, it will simply show an error and do nothing.
      The queue will be processed eventually and all the queued messages will be sent.
	
   * Set Device Profile:
      The app sets the device profile fields like link, profile id, window count, sync & async intervals for communication.

   * Set fake battery level:
      It sets a simulated battery level.

   * Enter DFU state:
	  This action disables Sidewalk stack and starts BLE SMP Server.
	  You can update a new firmware image using nRF Connect for mobile application.
	  To exit DFU state perform power cycle on your device.

.. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/boards/arm/nrf52dk_nrf52832/doc/index.html#nrf52dk-nrf52832

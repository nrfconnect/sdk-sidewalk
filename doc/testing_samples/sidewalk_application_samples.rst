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

   sidewalk_application_samples/ble_only.rst
   sidewalk_application_samples/template.rst

The table below shows differences between their configurations:

+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Criteria              | Template                                                      | Bluetooth LE only                          |
+=======================+===============================================================+============================================+
| Hardware required     | * `nRF52840dk_nrf52840`_                                      | * `nRF52840dk_nrf52840`_                   |
|                       | * External flash (included in the DK)                         |                                            |
|                       | * Semtech sx1262 (only for the configurations with LoRa/FSK)  |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Bootloader            | * MCUBoot - DFU partition in external flash                   | * MCUBoot - DFU partition in internal flash|
|                       | * Application partition size = 956kB                          | * Application partition size = 480kB       |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Application overlays  | * :file:`lora.conf`                                           | * :file:`usb_dfu`                          |
|                       | * :file:`fsk.conf`                                            |                                            |
|                       | * :file:`usb_dfu`                                             |                                            |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+
| Persistant data size  | * 8kB Zephyr settings                                         | * 8kB Zephyr settings                      |
|                       | * 28kB Sidewalk key-value storage                             | * 24kB Sidewalk key-value storage          |
+-----------------------+---------------------------------------------------------------+--------------------------------------------+

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

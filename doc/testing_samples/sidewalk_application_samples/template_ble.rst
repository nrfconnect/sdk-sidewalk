.. _template_ble:

Template Bluetooth LE
#####################

The sample demonstrates a template for Sidewalk End Node application.
It is optimized for Bluetooth LE.

.. _template_ble_requirements:

Requirements
************

The sample supports the following development kits:

+--------------------+----------+------------------------+------------------------------+
| Hardware platforms | PCA      | Board name             | Build target                 |
+====================+==========+========================+==============================+
| nRF52840 DK        | PCA10056 | `nrf52840dk_nrf52840`_ | ``nrf52840dk_nrf52840``      |
+--------------------+----------+------------------------+------------------------------+
| nRF5340 DK ``*``   | PCA10095 | `nrf5340dk_nrf5340`_   | ``nrf5340dk_nrf5340_cpuapp`` |
+--------------------+----------+------------------------+------------------------------+

``*`` - Experimental support

.. _template_ble_overview:

Overview
********

The sample shows implementation of the Sidewalk API for the Bluetooth LE transport protocol.
It is a memory-optimized example of Sidewalk configuration where only Bluetooth LE transport protocol is supported.
Because of the smaller footprint, both bootloader partitions for the application are placed in the internal flash memory of the supported SoC (nRF52840).

.. _template_ble_ui:

User interface
**************

A button action is triggered when you release the button.
To use a long press action, hold a button for 2 seconds or longer, and release it.

The button assignment is as follows:

   * Button 1 (long press):
      Factory Reset - The application informs the Sidewalk stack about the factory reset event.
      The Sidewalk library clears its configuration from the non-volatile storage.
      After a successful reset, the device needs to be registered with the cloud services again.

   * Button 2:
      Toggle Connection Request - The device requests the Sidewalk Gateway to initiate a connection while the device is advertising through Bluetooth LE.
      After the connection is dropped, the user has to set the beacon state again.
      Gateways may not always be able to process this request, as it depends on the number of devices connected to it.

   * Button 3:
      Send Hello - This action will queue a message to the cloud.
      If Sidewalk is not ready, it will simply show an error without sending the message.
      The queue will be processed eventually, and all the queued messages will be sent.

   * Button 4 (short press):
      Set fake battery level - The action sets a simulated battery level.

   * Button 4 (long press):
      Enter DFU state - This action disables the Sidewalk stack and starts the Bluetooth LE SMP Server.
      You can update the firmware image using nRF Connect for mobile application.
      To exit DFU state, perform a power cycle on your device.


LEDs represent the current state of the application:

   * LED 1:
      Application ``Connected`` successfully.

   * LED 2:
      Application ``Registered`` successfully.

   * LED 3:
      Application ``time sync`` successful.

   * LED 4:
      Application ``link`` is up.

Other states are routed to generic GPIO pins.
When the voltage drops to 0, entering low logic state, it means that one of the following states is active:

   * P1.1:
      ``DFU`` state.

   * P1.2:
      ``ERROR`` has occured.

   * P1.3:
      ``Sending`` pin is active when the packet is being transmitted.

   * P1.4:
      The ``Received`` pin is toggled ``on`` and immediately ``off`` when the packet has been received.

.. _template_ble_config:

Configuration
*************

When running the sample, you can use different configurations files depending on the supported features.

.. _template_ble_build_type:

Build types
===========

Configuration files are provided for different build types and are located in the application root directory.

The :file:`prj.conf` file represents a ``debug`` build type.
Other build types are covered by dedicated files with the respective build type added to the file name.
For example, the ``release`` build type file is named :file:`prj_release.conf`.
The same naming convention is followed if a board has other configuration files that are, for example, associated with the partition layout or child image configuration.

Before you start testing the application, you can select one of the build types supported by the sample.
Depending on the selected board, a sample supports the following build types:

* ``debug`` -- Debug version of the application - can be used to enable additional features, such as logs or command-line shell, to verify the application behavior.
* ``release`` -- Release version of the application - can be used to enable only the necessary application functionalities to optimize its performance.

You can build the ``release`` firmware for ``build_target`` by running the following command in the project directory:

.. parsed-literal::
   :class: highlight

   $ west build -b *build_target* -- -DCONF_FILE=prj_release.conf

For example:

.. code-block:: console

   $ west build -b nrf52840dk_nrf52840 -- -DCONF_FILE=prj_release.conf

.. note::
    Selecting a build type is optional.
    However, if the build type is not selected, the ``debug`` build type is used by default.

Configuration files
===================

For this sample, you can use the following overlays:

* :file:`cli` - This configuration adds an nRF Connect Shell over with Sidewalk Command Line Interface.

Bootloader and Device Firmware Update (DFU)
===========================================

MCUboot is configured to have two banks of memory for an application.

For more information on bootloaders and delivery options for the updated images, see `Bootloader and DFU solutions for NCS`_.

Memory layout
-------------

The following table presents how the memory is allocated for the Template Bluetooth LE sample:

    .. code::

        flash_primary (0x00000000 - 0x000fffff):
      +-------------------------------------------------------+
      | 0x00000000 - 0x00006fff: mcuboot (28kB)               |
      +---mcuboot_primary (480kB)-----------------------------+
      | 0x00007000 - 0x000071ff: mcuboot_pad (512B)           |
      | 0x00072000 - 0x0007efff: mcuboot_primary_app (479.5kB)|
      +-------------------------------------------------------+
      | 0x0007f000 - 0x000f6fff: mcuboot_secondary (480kB)    |
      | 0x000f7000 - 0x000f8fff: settings_storage (8kB)       |
      | 0x000f9000 - 0x000fefff: sidewalk_storage (24kB)      |
      | 0x000ff000 - 0x000fffff: mfg_storage (4kB)            |
      +-------------------------------------------------------+

        sram_primary (0x20000000 - 0x2003ffff):
      +-------------------------------------------------------+
      | 0x20000000 - 0x2003ffff: sram_primary (256kB)         |
      +-------------------------------------------------------+

DFU services
------------
To test the Device Firmware Update, follow the Testing Device Firmware Update (DFU) subsection in :ref:`template_ble_testing`.

If a new, correct image is found, the bootloader can only start the application that is placed in the first partition and exchange the application from the second partition.
It may take a few seconds.

Building and running
********************

This sample can be found under :file:`samples/template_ble`.

.. note::
   Before you flash you Sidewalk sample, make sure you completed the following:

      * You downloaded the Sidewalk repository and updated west according to the :ref:`dk_building_sample_app` section.
      * You provisioned your device during the :ref:`setting_up_sidewalk_product`.

   This step needs to be completed only once.
   You do not have to repeat it on every sample rebuild.

To build the sample, follow the steps in the `Building and programming an application`_ documentation.
If you want to select a specific build type instead of a default one, see :ref:`sensor_monitoring_selecting_build`

.. _template_ble_selecting_build:

Selecting a build type
======================

Before you start testing the application, you can select one of the available build types.
To select the build type when building the application from command line, specify it by adding one of the following parameters to the ``west build`` command:

   * Build with the default configuration:

      .. parsed-literal::
         :class: highlight

         $ west build -b *build_target*

   * Build with the release configuration:

      .. parsed-literal::
         :class: highlight

         $ west build -b *build_target* -- -DCONF_FILE=prj_release.conf

   * Build with the debug configuration with CLI

      .. parsed-literal::
         :class: highlight

         $ west build -b *build_target* -- -DCONFIG_SIDEWALK_CLI=y

.. _template_ble_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
To refresh the logs, restart the program by pressing the :guilabel:`RESET` button on your development kit.
You should see the following output:

      .. code-block:: console

         *** Booting Zephyr OS build v3.2.99-ncs2 ***
         ----------------------------------------------------------------
         sidewalk             v1.14.3-1-g1232aabb
         nrf                  v2.3.0
         zephyr               v3.2.99-ncs2
         ----------------------------------------------------------------
         sidewalk_fork_point = af5d608303eb03465f35e369ef22ad6c02564ac6
         build time          = 2023-03-14 15:00:00.000000+00:00
         ----------------------------------------------------------------

         [00:00:00.006,225] <inf> sid_template: Sidewalk example started!

Wait for the device to complete the :ref:`automatic_registration_sidewalk_endpoints`.

.. include:: testing_sample_include.txt

.. include:: ../../ncs_links.rst

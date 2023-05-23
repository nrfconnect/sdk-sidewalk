Sidewalk test simple bootloder application
##########################################

Application overview
********************

This is a simple application to be updated via sidewalk dfu process.
It is significantly smaller then any Sidewalk sample.

Maintenace notes
================
.. note::
   This appclication is designed to wrok with sidewalk template_ble sample

Configuration:
   * prj.conf - From Kconfig.dependencies config SIDEWALK_DFU_BOOTLOADER
   * pm_static.yaml - coppied from template_ble. Setting storage added as static partition (values from ninja partition_manager_report) to keep application footprint small.

Requirements
************

This application requires:
* nrf52840 DK
* mcumgr tool (cli or mobile app)

Building and running
********************

Build using west build.

Testing
=======

Precondition
#. Build and flash Sidewalk template_ble sample
#. Flash MFG.hex (provisoing data)
#. Enter DFU mode (button action)

Testing sample bootloder application
#. Build application
#. Ready to upload image is generated in sidewalk/tests/manual/simple_bootloader/build/zephyr/app_update.bin
#. upload image using mcumgr tool, mark to test, reset device
#. wait or bootloder to swap images
#. Application marks image as ok and prints message

Application output
==================

The application shows the following output:

.. code-block:: console

   [00:00:00.407,623] <inf> main: Marked image as OK
   [00:00:00.407,623] <inf> main: Hello world!

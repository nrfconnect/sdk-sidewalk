.. image:: https://github.com/nrfconnect/sdk-sidewalk/actions/workflows/on-commit.yml/badge.svg?branch=main
 :target: https://github.com/nrfconnect/sdk-sidewalk/actions/workflows/on-commit.yml

nRF Connect SDK: sdk-sidewalk
#############################

This repository contains an Amazon Sidewalk platform abstraction layer for the Nordic nRF Connect SDK which is based on Zephyr RTOS.

This branch added experimental support for RAK4631 board.

RAK4631
*******

This branch supports `rak4631dk/nrf52840` board in Sidewalk End Device sample in variants Hello, and Device Under Test.

Limitations
-----------

* DFU (MCUBoot) is not supported
* Device needs to be flashed via external SWD
* LoRa/FSK connection is unstable

Setup
-----

For RAK4631 setup instruction see: https://docs.rakwireless.com/product-categories/wisblock/rak4631/sidewalk/#device-setup

Build and test
--------------

To build sample use standard ``west build``:

.. code-block:: console

    cd sidewalk/samples/sid_end_device
    west build -b rak4631dk/nrf52840


To program with the RAK4631 board, connect to the board using SWD interface and run the flowing commands instead of ``west flash``:

.. code-block:: console

    pyocd erase -t nrf52840 --chip
    pyocd flash -t nrf52840 build/merged.hex
    pyocd flash -t nrf52840 mfg.hex
    pyocd reset -t nrf52840

To see the RAK4631 logs, connect the board USB interface and found USB device named `Nordic_Semiconductor_`:

.. code-block:: console

    ls -l /dev/serial/by-id/
    usb-ARM_DAPLink_CMSIS-DAP_01223456789-if01 -> ../../ttyACM1
    usb-Nordic_Semiconductor__01ABC-if00 -> ../../ttyACM0
    screen /dev/ttyACM0 115200

After successful connection, the following logs are expected:

.. code-block:: console

    *** Booting Sidewalk v0.1.99-addon-7f87292e247a ***
    *** Using nRF Connect SDK v3.0.0-3bfc46578e42 ***
    *** Using Zephyr OS v4.0.99-a0e545cb437a ***
    ----------------------------------------------------------------
    Sidewalk SDK        = 1.18.0.18
    APP_BUILD_VERSION   = v1.0.0-add-on-3-g7f87292e247a
    APP_NAME            = dut
    build time          = May 15 2025 17:00:00
    board               = rak4631/nrf52840
    ----------------------------------------------------------------
    uart:~$ 


More about RAK4631 board: https://docs.nordicsemi.com/bundle/ncs-3.0.0/page/zephyr/boards/rakwireless/rak4631/doc/index.html

Documentation
*************

Browse the official Sidewalk documentation:

* To see the official nRF Connect SDK documentation, go to https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/amazon_sidewalk/index.html

* Amazon Sidewalk: https://docs.sidewalk.amazon

Support
*******

To ask questions about Sidewalk in the nRF Connect SDK, refer to https://devzone.nordicsemi.com.

.. _registration_over_wire:

Registration over Wire
######################

.. contents::
   :local:
   :depth: 2

Registration over Wire (ROW) registers a Sidewalk Endpoint at the factory over a serial connection, without a Bluetooth® LE registration flow.
A device registered this way is factory-registered.
The Amazon Sidewalk stack field-activates the device on its first contact with the network.

The factory line uses two images:

* The factory diagnostic firmware, which runs ROW.
  It is the Device Under Test (DUT) variant of the :ref:`Sidewalk end device <sidewalk_end_device>` sample with the ROW overlay.
* The production firmware, which the device ships with.
  It contains no ROW support.

.. note::
   The add-on supports ROW with the full Sidewalk libraries at the high security level.
   The single-link lite build variants are not supported.

Overview
********

The factory diagnostic firmware reaches the line PC over one UART, the ``zephyr,shell-uart`` chosen node.
The UART carries two kinds of traffic in both directions:

* Text: shell command lines, prompt, echo, log lines, and the ``{CERT ...}`` and ``{REGISTRATION ...}`` replies.
* Binary frames: ``0xAA | LEN (1 byte) | payload``, with a payload of 1 to 255 bytes.
  The payloads are encrypted Sidewalk registration messages.
  The firmware only frames and deframes them.

A dedicated shell backend owns the UART.
It separates the frames from the console text on the first byte of each unit, and it writes every frame as one unit, so that no log line is inserted into it.
Console text is 7-bit ASCII, so the ``0xAA`` sync byte never occurs in it.

The wired Sidewalk link (``SID_LINK_TYPE_8``) is brought up by the serial registration API, not by the :c:func:`sid_init` and :c:func:`sid_start` functions.
Do not include ``SID_LINK_TYPE_8`` in the ``link_mask`` field of the Sidewalk configuration.

Configuration
*************

The :kconfig:option:`CONFIG_SIDEWALK_REGISTRATION_OVER_WIRE` Kconfig option enables ROW.
It requires the :kconfig:option:`CONFIG_SIDEWALK_ON_DEV_CERT` Kconfig option, and it is not compatible with the default serial shell backend and the UART console.

The ``overlay-row.conf`` file of the Sidewalk end device sample sets the required options.
Apply it on top of the DUT overlay:

.. code-block:: console

   $ west build -b nrf54l15dk/nrf54l15/cpuapp -- -DOVERLAY_CONFIG="overlay-dut.conf;overlay-row.conf"

The following Kconfig options tune the ROW transport:

* :kconfig:option:`CONFIG_SIDEWALK_ROW_CONSOLE_RING_SIZE` -- Size of the receive buffer for the console text.
* :kconfig:option:`CONFIG_SIDEWALK_ROW_POLL_INTERVAL_MS` -- Interval of the registration processing while the registration is active.
* :kconfig:option:`CONFIG_SIDEWALK_ROW_SHELL_LOG_QUEUE_SIZE` and :kconfig:option:`CONFIG_SIDEWALK_ROW_SHELL_LOG_QUEUE_TIMEOUT` -- Log message queue of the ROW shell backend.

The UART runs at 115200 baud, 8N1, with no flow control.

Shell commands
**************

The ``cert`` commands provision the device certificates, as in the on-device certificate generation flow.
The ``register`` commands drive the factory registration.
Each ``register`` command answers with one line: the line listed below on success, or ``{REGISTRATION ERROR <n>}`` on failure, where ``<n>`` is a ``sid_error_t`` value.

.. list-table:: Registration commands
   :header-rows: 1
   :widths: 25 50 25

   * - Command
     - Description
     - Reply
   * - ``register start``
     - Brings up the registration stack and the wired link, and starts the registration exchange.
       The device then sends the first ``0xAA`` frames.
     - ``{REGISTRATION STARTED}``
   * - ``register get_state``
     - Prints the device registration state: ``0`` not registered, ``1`` registered, ``2`` factory-registered.
     - ``{REGISTRATION STATE=<n>}``
   * - ``register commit_keys``
     - Persists the registration credentials from the key-value store into the manufacturing page.
       The host tool sends it only at the low security level.
     - ``{REGISTRATION KEYS_COMMITTED}``
   * - ``register finalize``
     - Tears down the registration stack and the wired link.
     - ``{REGISTRATION FINALIZED}``

The host tool drives the registration and determines its completion from the Amazon Sidewalk cloud.
The following rules apply:

* ROW and the Sidewalk stack are mutually exclusive.
  The ``register start`` command fails if the Sidewalk stack is initialized, and the ``sid init`` command fails until ``register finalize`` is run.
* A second ``register start`` without ``register finalize`` fails with the ``SID_ERROR_ALREADY_INITIALIZED`` error.
* A failed attempt leaves the registration stack initialized.
  Run ``register finalize`` or reset the device before you retry.

Bring-up
********

Bring up the integration in the following order:

#. Send ``help`` from a serial terminal and check that the ``cert`` and ``register`` commands are listed.
   Close the terminal before you run the host tool, because the serial port has one reader.
#. Provision the device certificates with the host tool.
   The device answers each ``cert`` command with ``{CERT ...}`` lines.
#. Register the device with the host tool.
   The device answers ``register start`` with ``{REGISTRATION STARTED}`` and then sends the first ``0xAA`` frames.
#. Send ``help`` again.
   This detects console problems that appear only after binary traffic.

Production firmware
*******************

The production firmware contains no ROW support.
It must use a flash layout compatible with the factory diagnostic firmware:

* The same manufacturing partition address.
* The same key-value storage partition, address and size, and the same Sidewalk key-value group IDs.

A layout mismatch is not detected at the factory.
It appears in the field as an unregistered device.
Verify each production firmware release by flashing it on a factory-registered device and confirming the field activation.

The manufacturing store keeps the registration credentials in the ``SID_PAL_MFG_STORE_*`` values 4001 to 4008, so that a factory-registered device keeps its registration across a reflash of the key-value storage.
Enable the :kconfig:option:`CONFIG_SIDEWALK_MFG_CREDENTIALS_RECOVER` Kconfig option in the production firmware.
The :ref:`Sidewalk end device <sidewalk_end_device>` sample then calls the ``sid_mfg_credentials_recover()`` function at boot, before the Sidewalk stack is initialized.
The function copies the credentials from the manufacturing store into the key-value storage, if the key-value storage does not already hold them.

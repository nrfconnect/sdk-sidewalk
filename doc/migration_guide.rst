.. _migration_guide:

Migration guide
***************

.. contents::
   :local:
   :depth: 2

Starting with the v2.6.0 release of the `nRF Connect SDK`_, a new sample model has been introduced.
For an overview of changes, read the following sections.

Reasons for change
==================

The new sample model allows users to easily:

* Adopt new features from the Amazon Sidewalk and nRF Connect SDK.
* Maintain samples codebase.
* Understand how the application works.

The old Amazon Sidewalk samples model in the nRF Connect SDK was divided into four different samples: Template sub-GHz, Template Bluetooth® LE, Sensor monitor and Device under test (dut).
The new approach allows to present the same functionalities through one, unified, and configurable :ref:`sidewalk_end_device`.

.. tabs::
    .. tab:: New model for samples

        Introduces common, configurable code base:

        .. figure:: images/unify_sample_new.svg
           :scale: 100 %
           :alt: New samples model

    .. tab:: Old model for samples

        Samples are separate for each configuration:

        .. figure:: images/unify_sample_old.svg
           :scale: 100 %
           :alt: Old samples model

.. note::
    To make sure your application works without issues, it is recommended to align it to the new sample.
    In general, you can still use applications that are based on the old samples.
    However, beside the sample unification, the ``sdk-sidewalk`` codebase has undergone multiple changes that might affect your code.

Aligning your application to the new model
==========================================

To understand how your application might be affected by the new sample model, see the following list of changes.

Building
--------

The following table is an example of the new build commands used for the `nRF52840 DK`_ with the :file:`{NCS_PATH}/sidewalk/samples` path.

+----------------------------+---------------------------------------------------------+----------------------------------------------------------------------------------------------+
| Sample                     | Old commands                                            | New commands                                                                                 |
+============================+=========================================================+==============================================================================================+
| Template sub-GHz           | ``west build -b nrf52840dk_nrf52840 template_subghz``   | ``west build -b nrf52840dk_nrf52840 sid_end_device``                                         |
+----------------------------+---------------------------------------------------------+----------------------------------------------------------------------------------------------+
| Template Bluetooth LE only | ``west build -b nrf52840dk_nrf52840 template_ble``      | ``west build -b nrf52840dk_nrf52840 sid_end_device -- -DCONFIG_SIDEWALK_SUBGHZ_SUPPORT=y``   |
+----------------------------+---------------------------------------------------------+----------------------------------------------------------------------------------------------+
| Sensor monitoring demo     | ``west build -b nrf52840dk_nrf52840 sensor_monitoring`` | ``west build -b nrf52840dk_nrf52840 sid_end_device -- -DOVERLAY_CONFIG="overlay-demo.conf"`` |
+----------------------------+---------------------------------------------------------+----------------------------------------------------------------------------------------------+
| Device under test          | ``west build -b nrf52840dk_nrf52840 sid_dut``           | ``west build -b nrf52840dk_nrf52840 sid_end_device -- -DOVERLAY_CONFIG="overlay-dut.conf"``  |
+----------------------------+---------------------------------------------------------+----------------------------------------------------------------------------------------------+

User interface
--------------

The user interface was changed to better support boards with a smaller amount of buttons and LEDs.

The following table shows button action differences between the old template samples and new :ref:`sidewalk_end_device` application in the :ref:`variant_sidewalk_hello` variant:

+--------------------------------+--------------------------+-------------------------+
| Button action                  | Old mapping              | New mapping             |
+================================+==========================+=========================+
| Send Hello                     | ``Button 3 (short)``     | ``Button 1 (short)``    |
+--------------------------------+--------------------------+-------------------------+
| Enter DFU mode                 | ``Button4 (long)``       | ``Button 1 (long)``     |
+--------------------------------+--------------------------+-------------------------+
| Factory Reset                  | ``Button1 (long)``       | ``Button 2 (long)``     |
+--------------------------------+--------------------------+-------------------------+
| Board support                  | ``board_events.c``       | N/A (use CLI command)   |
+--------------------------------+--------------------------+-------------------------+
| Set fake battery level         | ``Button 4 (short)``     | N/A (use CLI command)   |
+--------------------------------+--------------------------+-------------------------+
| Get current Device Profiles    | ``Button 2 (short)``     | N/A (use CLI command)   |
+--------------------------------+--------------------------+-------------------------+
| Switch between Device Profiles | ``Button 2 (long)``      | N/A (use CLI command)   |
+--------------------------------+--------------------------+-------------------------+
| Switch Link Mask               | N/A                      | ``Buttons 3 (long)``    |
+--------------------------------+--------------------------+-------------------------+

The following table shows LED assignment differences between the old template samples and new :ref:`sidewalk_end_device` application in the :ref:`variant_sidewalk_hello` variant:

+--------------------------------+--------------+--------------+
| LED status                     | Old          | New          |
+================================+==============+==============+
| Application Link and Connected | ``LED 1``,   | ``LED 1``    |
|                                | ``LED 4``    |              |
+--------------------------------+--------------+--------------+
| Application time sync          | ``LED 3``    | ``LED 2``    |
+--------------------------------+--------------+--------------+
| Application Registered         | ``LED 2``    | ``LED 3``    |
+--------------------------------+--------------+--------------+
| Application woke up            | N/A          | ``LED 4``    |
+--------------------------------+--------------+--------------+

Additionally, the following CLI changes have been made:

* All Sidewalk Device Under Test application (``sid_dut``) commands are available when the ``CONFIG_SID_END_DEVICE_CLI`` option is enabled.
* The old ``CONFIG_SIDEWALK_CLI`` option was removed.

Source files
------------

The file structure was refactored for the sample unification purposes.
The following table shows the relocation of most of the components:

+--------------------------------+------------------------------+------------------------+
| Component                      | Old location                 | New location           |
+================================+==============================+========================+
| Sidewalk thread implementation | :file:`application_thread.c` | :file:`sidewalk.c`     |
+--------------------------------+------------------------------+------------------------+
| Sidewalk thread events         | :file:`application_thread.h` | :file:`sidewalk.h`     |
+--------------------------------+------------------------------+------------------------+
| Sidewalk callbacks             | :file:`sidewalk_callbacks.c` | :file:`app.c`          |
+--------------------------------+------------------------------+------------------------+
| Board support                  | :file:`board_events.c`       | :file:`app.c`          |
+--------------------------------+------------------------------+------------------------+
| Application logic              | :file:`main.c`               | :file:`app.c`          |
+--------------------------------+------------------------------+------------------------+

Events
------

In the nRF Connect SDK, you must make all Sidewalk API calls form the same thread where ``sid_process()`` is called.
In case of both old and new samples, communication with the Sidewalk thread is handled by the user-defined events.
To cover all scenarios, the following updates have been made:

* Added more events.
* Renamed other events.
* An event can now provide Sidewalk thread a specific context.

+--------------------------------+-------------------------------------+-----------------------------+
| Event                          | Old name                            | New name                    |
+================================+=====================================+=============================+
| Sidewalk process event         | ``SIDEWALK_EVENT``                  | ``SID_EVENT_SIDEWALK``      |
+--------------------------------+-------------------------------------+-----------------------------+
| Send message                   | ``BUTTON_EVENT_SEND_HELLO``         | ``SID_EVENT_SEND_MSG``      |
+--------------------------------+-------------------------------------+-----------------------------+
| Set connection request         | ``BUTTON_EVENT_CONNECTION_REQUEST`` | ``SID_EVENT_CONNECT``       |
+--------------------------------+-------------------------------------+-----------------------------+
| Factory reset                  | ``BUTTON_EVENT_FACTORY_RESET``      | ``SID_EVENT_FACTORY_RESET`` |
+--------------------------------+-------------------------------------+-----------------------------+
| Enter DFU mode                 | ``BUTTON_EVENT_NORDIC_DFU.c``       | ``SID_EVENT_NORDIC_DFU``    |
+--------------------------------+-------------------------------------+-----------------------------+
| Set battery option,            | ``BUTTON_EVENT_SET_BATTERY_LEVEL``  | ``DUT_EVENT_SET_OPTION``    |
| Set device profile option      | ``BUTTON_EVENT_SET_DEVICE_PROFILE`` |                             |
+--------------------------------+-------------------------------------+-----------------------------+
| Get device profile option      | ``BUTTON_EVENT_GET_DEVICE_PROFILE`` | ``DUT_EVENT_GET_OPTION``    |
+--------------------------------+-------------------------------------+-----------------------------+
| New events                     | N/A                                 | ``SID_EVENT_LINK_SWITCH``   |
|                                |                                     | ``SID_EVENT_NEW_STATUS``    |
|                                |                                     | ``DUT_EVENT_INIT``          |
|                                |                                     | ``DUT_EVENT_DEINIT``        |
|                                |                                     | ``DUT_EVENT_START``         |
|                                |                                     | ``DUT_EVENT_STOP``          |
|                                |                                     | ``DUT_EVENT_GET_MTU``       |
|                                |                                     | ``DUT_EVENT_GET_TIME``      |
|                                |                                     | ``DUT_EVENT_GET_STATUS``    |
|                                |                                     | ``DUT_EVENT_SET_DEST_ID``   |
+--------------------------------+-------------------------------------+-----------------------------+

The following examples show code differences for selected events:

* Sending event procedure (example of sending the ``hello`` message to cloud).
  Part of the code was removed to improve its readability.

    .. tabs::
        .. tab:: New code

            .. code:: c

                // app.c
                const char payload[] = "hello";
                sidewalk_msg_t *hello = sid_hal_malloc(sizeof(sidewalk_msg_t));
                hello->msg.size = sizeof(payload);
                hello->msg.data = sid_hal_malloc(hello->msg.size);
                memcpy(hello->msg.data, payload, hello->msg.size);
                hello->desc.type = SID_MSG_TYPE_NOTIFY;
                hello->desc.link_type = SID_LINK_TYPE_ANY;
                hello->desc.link_mode = SID_LINK_MODE_CLOUD;

                sidewalk_event_send(SID_EVENT_SEND_MSG, hello);

                // sidewalk.c
                switch (sm->event.id) {
                case SID_EVENT_SEND_MSG:
                    sidewalk_msg_t *p_msg = (sidewalk_msg_t *)sm->event.ctx;
                    sid_put_msg(sm->sid->handle, &p_msg->msg, &p_msg->desc);
                    sid_hal_free(p_msg->msg.data);
                    sid_hal_free(p_msg);
                    break;
                }

        .. tab:: Old code

            .. code:: c

                // main.c
                app_event_send(BUTTON_EVENT_SEND_HELLO);

                // application_thread.c
                switch (event) {
                case BUTTON_EVENT_SEND_HELLO:
                    button_event_send_hello(application_ctx);
                    break;
                }

                // board_events.c
                void button_event_send_hello(app_ctx_t *application_ctx)
                {
                    const char payload[] = "hello";
                    struct sid_msg msg;
                    msg.data = &payload;
                    msg.size = sizeof(payload);
                    struct sid_msg_desc desc;
                    desc.type = SID_MSG_TYPE_NOTIFY;
                    desc.link_type = SID_LINK_TYPE_ANY;
                    desc.link_mode = SID_LINK_MODE_CLOUD;

                    sid_put_msg(application_ctx->handle, &msg, &desc);
                }

* Handling event procedure (example of handling the Sidewalk process event).
  Part of the code was removed to improve its readability.

    .. tabs::
        .. tab:: New code

            .. code:: c

                // context
                typedef struct {
                    struct sid_handle *handle;
                    struct sid_config config;
                    struct sid_status last_status;
                } sidewalk_ctx_t;

                typedef struct {
                    struct smf_ctx ctx;
                    struct k_msgq msgq;
                    sidewalk_ctx_event_t event;
                    sidewalk_ctx_t *sid;
                } sm_t;

                // init
                static void state_sidewalk_entry(void *o)
                {
                    sm_t *sm = (sm_t *)o;
                    sid_init(&sm->sid->config, &sm->sid->handle);
                    sid_start(sm->sid->handle, sm->sid->config.link_mask);
                }

                // process
                static void state_sidewalk_run(void *o)
                {
                    sm_t *sm = (sm_t *)o;

                    switch (sm->event.id) {
                        case SID_EVENT_SIDEWALK:
                            sid_process(sm->sid->handle);
                        break;
                    }
                }

                static void sid_thread_entry(void *context, void *unused, void *unused2)
                {
                    while (true) {
                        k_msgq_get(&sid_sm.msgq, &sid_sm.event, K_FOREVER);
                        smf_run_state(SMF_CTX(&sid_sm))
                    }
                }

                // thread start
                void sidewalk_start(sidewalk_ctx_t *context)
                {
                    (void)k_thread_create(sid_thread_entry, context);
                }


                // event send
                int sidewalk_event_send(sidewalk_event_t event, void *ctx)
                {
                    sidewalk_ctx_event_t ctx_event = {
                        .id = event,
                        .ctx = ctx,
                    };

                    return k_msgq_put(&sid_sm.msgq, (void *)&ctx_event, K_NO_WAIT);
                }

        .. tab:: Old code

            .. code:: c

                // context
                typedef struct application_context {
                    struct sid_event_callbacks event_callbacks;
                    struct sid_config config;
                    struct sid_handle *handle;
                } app_ctx_t;


                static void sidewalk_app_entry(void *ctx, void *unused, void *unused2)
                {
                    // init
                    sid_init(&application_ctx->config, &application_ctx->handle);
                    sid_start(application_ctx->handle, BUILT_IN_LM);

                    // process
                    while (true) {
                        app_event_t event = SIDEWALK_EVENT;

                        if (!k_msgq_get(&application_thread_msgq, &event, K_FOREVER)) {
                            switch (event) {
                                case SIDEWALK_EVENT:
                                    sid_process(application_ctx->handle);
                            }
                        }
                    }
                }

                // thread start
                sid_error_t app_thread_init(app_ctx_t *context)
                {
                    k_thread_create(sidewalk_app_entry, context);

                }

                // event send
                void app_event_send(app_event_t event)
                {
                    k_msgq_put(&application_thread_msgq, (void *)&event, K_NO_WAIT);
                }

.. _migration_guide_dfu_mode:

Sidewalk and custom Bluetooth Service
-------------------------------------

:ref:`sidewalk_end_device` uses the `Zephyr State Machine Framework`_.
The :file:`sidewalk.c` file uses the state machine to demonstrate how the application can switch between Sidewalk and the `Zephyr SMP Server sample`_.
You can use this as a reference design for switching between Sidewalk mode and another Bluetooth LE service-based application.

.. include:: ncs_links.rst

Using old samples model
=======================

To use the deprecated functionalities, you have to enable the relevant Kconfig options:

* ``CONFIG_DEPRECATED_SIDEWALK_PAL_INIT`` -- The option initializes platform components from the :file:`pal_init.c` file.
  The initialization is moved to the :file:`sid_common.c` file and is called during ``sid_init()``.
* ``DEPRECATED_SIDEWALK_BLE_MAC_ADDR_TYPE`` -- The option switches the Bluetooth LE Mac address type for the Sidewalk application.
  These configurations are redundant to the nRF Connect SDK Bluetooth configurations.
* ``DEPRECATED_DFU_FLAG_SETTINGS_KEY`` -- The option saves the DFU mode after reset.
  The new sample uses the :ref:`migration_guide_dfu_mode`.
* ``CONFIG_SIDEWALK_LINK_MASK`` -- The option chooses sidewalk link mask to start with.
  New Sidewalk libraries support link switch in runtime.
  To switch between the full link mask support libraries and Bluetooth LE only libraries, use the ``CONFIG_SIDEWALK_SUBGHZ_SUPPORT`` option.

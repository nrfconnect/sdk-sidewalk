.. _migration_guide_v280:

Migration guide for the v2.8.0 of the nRF Connect SDK
*****************************************************

.. contents::
   :local:
   :depth: 2

Starting with the v2.8.0 release of the `nRF Connect SDK`_, there is a new Sidewalk provisioning module.
For an overview of changes, read the following sections.

Reasons for change
==================

The new provisioning module enhances security by:

* Moving non-volatile Sidewalk keys to secure key storage.
* Protecting the manufacturing data partition from write operations.
* Always using the Key-Value Store version of manufacturing data.

Security enhancements details
=============================

Access to the manufacturing data is unified within the application, ensuring consistency for prototyping and manufacturing flows.
The MFG parser module is launched at initialization and writes the manufacturing data to flash (secure key storage) in the same way as the device certification module.
The process is triggered only once, at the first application start after flashing the :file:`Nordic_MFG.hex` file.
The following logs are generated:

.. code-block:: console

   [00:00:00.004,502] <inf> sid_mfg: Need to parse mfg data
   [00:00:00.009,993] <inf> sid_mfg_parser_v8: MFG_ED25519 import success
   [00:00:00.014,900] <inf> sid_mfg_parser_v8: MFG_SECP_256R1 import success
   [00:00:00.054,505] <inf> sid_mfg: Successfully parsed mfg data

The secure key storage is based on the `Hardware unique key`_ security library.
After keys are imported into secure key storage, it is impossible to read their raw values.
Therefore, once secure key storage is enabled in the firmware, it must remain enabled in all subsequent firmware versions. 
This feature is enabled by default in all Sidewalk samples.

The hardware flash write protection is based on the `Hardware flash write protection`_ security library.
The protection is applied during Sidewalk initialization and remains irreversible until reset.
This feature is disabled in the :ref:`variant_sidewalk_dut` sample to allow writing of manufacturing data using on-device certification commands.

Aligning your application to the new model
==========================================

The new provisioning module implementation includes the following changes:

* In the :file:`app_mfg_config.h` file, the function ``app_mfg_cfg_is_valid()`` was renamed to ``app_mfg_cfg_is_empty()``.

* The ``CONFIG_SIDEWALK_MFG_STORAGE_SUPPORT_HEX_v7`` Kconfig option adds support for older Sidewalk manufacturing HEX formats (version 7 and below). 
  You can disable this configuration when using the latest manufacturing flash.

Using old provisioning module
=============================

It is recommended to use the new provisioning module implementation. 
However, you can still enable the old module using the ``CONFIG_DEPRECATED_SIDEWALK_MFG_STORAGE`` Kconfig option. 

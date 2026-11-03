.. _setting_up_manufacturing_environment:

Setting up manufacturing environment
####################################

.. contents::
   :local:
   :depth: 2

You can set up manufacturing environment to produce large quantities of Amazon Sidewalk-enabled devices.
The process includes bulk production, provisioning, and onboarding to AWS IoT.
It also involves device authentication and managing the lifecycle of a Sidewalk device from prototyping to manufacturing.
For detailed instructions on setup and tooling, refer to the `Amazon Sidewalk Manufacturing Guide`_.

The Device Under Test (DUT) Command Line Interface (CLI) mentioned in the Amazon mass production flow is implemented by the :ref:`variant_sidewalk_dut` sample.
Additionally, you can enable the on-device certification CLI in any Sidewalk application by activating the ``CONFIG_SIDEWALK_ON_DEV_CERT`` Kconfig option.

Starting with the Amazon Sidewalk SDK v2.0 release, you can provision devices and register them on the factory line over a wired connection without any registration in the field (see `Amazon Sidewalk Registration over Wire Application Note`_):

* To run Registration over Wire (ROW) in the :ref:`variant_sidewalk_dut` sample, build it with the ``overlay-row.conf`` overlay.
  This overlay enables the experimental ``CONFIG_SIDEWALK_REGISTRATION_OVER_WIRE`` Kconfig option.
* In the production firmware, enable the experimental ``CONFIG_SIDEWALK_FACTORY_REGISTRATION_RESTORE`` Kconfig option and keep the manufacturing and key-value storage partitions at the same addresses as in the factory firmware.

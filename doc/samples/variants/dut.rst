.. _variant_sidewalk_dut:

Device Under Test (DUT)
#######################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

Configuration options
*********************

Before building the Device under test application variant of the Sidewalk end device, ensure you have enabled the ``OVERLAY_CONFIG="overlay-dut.conf"`` configuration.

.. include:: ../../includes/include_kconfig_common.txt

Building and running
********************

.. include:: ../../includes/include_building_and_running.txt
.. include:: ../../includes/include_rak4631_flash.txt

Testing
=======

#. After successfully building and flashing the sample is ready to use.
   You should see the following command-line prompt:

   .. code-block:: console

       uart$

#. To start testing the application, use the Sidewalk CLI (DUT).
   Type ``sid help`` and press enter to get the list of supported commands.

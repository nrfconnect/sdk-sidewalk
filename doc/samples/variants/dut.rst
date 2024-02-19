.. _sidewalk_dut:

Device Under Test
#################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

Configuration
*************

.. note::
    To build the Device under test application variant of the Sidewalk end device, use the ``OVERLAY_CONFIG="overlay-dut.conf"`` configuration.

For more configuration option see :ref:`sidewalk_end_device_configuration`.

Building and running
********************

.. include:: include_building_and_running.txt

Testing
*******

#. After successfully building and flashing the sample is ready to use. You should see the following command-line prompt:

   .. code-block:: console

       uart$

#. To start testing the application, use the Sidewalk CLI (DUT).
   Type ``sid help`` and press enter to get the list of supported commands.

.. include:: ../../ncs_links.rst

.. _device_under_test:

Device Under Test
#################

This sample demonstrates how to use Sidewalk Device Under Test for the testing and evaluation purposes.

.. _device_under_test_ui:

User interface
**************

Device uses UART CLI to communicate with the user.
To see the list of available commands, type ``sid help``.

.. _device_under_test_testing:

Testing
=======

See `Testing and debugging an application`_ for information on testing and debugging in the nRF Connect SDK.

After successfully building the sample and flashing manufacturing data, the sample is ready to use.
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
       uart:~$

To start testing the application, you can use the available commands (``sid help``).

.. include:: ../../ncs_links.rst

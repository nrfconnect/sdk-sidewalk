.. _sidewalk_testing:

Sidewalk Testing
################

Levels of Testing
=================

Testing levels are essential for a structured approach to quality assurance, allowing for targeted testing at each stage of development to reduce defects and ensure software reliability.

The most commonly recognized levels of testing in software development include:

* **Unit Testing**: Focuses on individual functions to ensure they work correctly in isolation.
* **Integration Testing**: Tests the interaction between integrated components to identify issues arising from their interactions.
* **System Testing**: Validates the complete and integrated software system against specified requirements.
* **Acceptance Testing**: Confirms that the software meets the business needs and is ready for deployment, often involving end-user validation.

.. note::

    There is sometimes a confusion between *integration* tests and *functional* tests as they both require multiple components to interact with each other.
    For the purposes of this document, *integration* term is used to describe tests that verify components, but not necessarily the system as a whole.

For System Testing and Acceptance Testing, you can use Sidewalk sample variants, such as:

* CLI - application to test against the Sidewalk Test Specification.
* Sensor monitoring - example of a sensor device use case, with release configuration support.

In this document we focus on Unit and Integration testing, as they are delivered as part of the sdk-sidewalk repository.

Table below provides a comparison of Unit and Integration testing in the Sidewalk SDK:

+----------------+---------------------+--------------------------+
|                | Unit                | Integration              |
+================+=====================+==========================+
| Scope          | One source file     | One module               |
+----------------+---------------------+--------------------------+
| Platform       | * unit_testing (PC) | * native_sim (PC)        |
|                |                     | * nRF development boards |
+----------------+---------------------+--------------------------+
| Frameworks     | * Ztest             | * Ztest                  |
|                | * Twister           | * Twister                |
|                | * FFF               |                          |
+----------------+---------------------+--------------------------+
| Allows mocking | Yes                 | No                       |
+----------------+---------------------+--------------------------+


Unit Testing
============

Unit tests are designed to test individual functions or methods in isolation to ensure they work as expected.

To run unit test:

#. Go to the unit test folder:

   .. code-block:: bash

      cd sidewalk/tests/unit/<test_name>

#. Build the test application by running the following command:

   .. code-block:: bash

      west build --no-sysbuild -b unit_testing

#. Run the test application by running the following command:

   .. code-block:: bash

      ./build/testbinary

Integration Testing
===================

Integration tests are designed to verify that multiple components work together as expected.

There are two main purposes of integration tests:

* Verify that multiple files and functions work together as one module, e.g. crypto, timer, storage. Those tests typically operates on Sidewalk Platform Abstraction Layer (PAL) API. Usually there is a dedicated Kconfig option to enable/disable the module.
* Verify that module works correctly on specify Nordic SoC. This type of tests was sometimes called "unit test on hardware".

To run integration test:

.. tabs::

    .. tab:: native_sim (PC)
    
        #. Go to the integration test folder:
    
            .. code-block:: bash
    
                cd sidewalk/tests/integration/<test_name>
    
        #. Build the test application by running the following command:
    
            .. code-block:: bash
    
                west build -b native_sim
    
        #. Run the test application by running the following command:
    
            .. code-block:: bash
    
                ./build/<test_name>/zephyr/zephyr.exe

    .. tab:: nRF (development boards)

        #. Go to the integration test folder:

            .. code-block:: bash

                cd sidewalk/tests/integration/<test_name>

        #. Build the test application by running the following command:

            .. code-block:: bash

                west build -b <board_name>

            Use your board name e.g. ``nRF52840dk/nrf52840``, ``nRF55l15dk/nrf54l15/cpuapp``.

        #. Flash the test application running the following command:

            .. code-block:: bash

                west flash
            
            Make sure your board is connected to the computer.
            Test output will be displayed on the UART console.

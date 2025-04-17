.. _sidewalk_testing:

SDK testing procedures
######################

.. contents::
   :local:
   :depth: 2

This following page provides an overview on the various levels of testing within the Amazon Sidewalk SDK.
Specifically, it is designed to show what tests are available and to provide detailed instructions on how to run them.

Testing procedures
******************

Testing levels are crucial for a structured approach to quality assurance, enabling you to conduct targeted testing at each stage of development.
This helps to reduce defects and ensures the reliability of your software.

The following are the most commonly recognized levels of testing in software development:

* Unit testing - Focuses on individual functions to ensure they work correctly in isolation.
* Integration testing - Tests the interaction between integrated components to identify issues arising from their interactions.
* System testing - Validates the complete and integrated software system against specified requirements.
* Acceptance testing - Confirms that the software meets the business needs and is ready for deployment, often involving end-user validation.

.. note::

    It is important to distinguish between *integration* tests and *functional* tests, as both involve multiple components interacting.
    In this document, the term *integration* is specifically used to describe tests that verify the components, but not necessarily the entire system.

For system testing and acceptance testing, you can use :ref:`Amazon Sidewalk sample variants <samples_list>`, such as:

* CLI - An application to test against the Amazon Sidewalk test specification.
* Sensor monitoring - An example of a sensor device use case, with release configuration support.

This document primarily addresses Unit and Integration testing provided in the `sdk-sidewalk repository`_.

The following table provides a comparison of Unit and Integration testing in the Amazon Sidewalk SDK:

.. list-table:: Testing Characteristics
   :widths: 25 25 50
   :header-rows: 1

   * - Test Attributes
     - Unit
     - Integration
   * - Scope
     - One source file
     - One module
   * - Platform
     - * unit_testing (PC)
     - * native_sim (PC)
       * nRF development boards
   * - Frameworks
     - * Ztest
       * Twister
       * FFF
     - * Ztest
       * Twister
   * - Allows mocking
     - Yes
     - No

Unit testing
============

Unit tests are designed to verify individual functions or methods in isolation, ensuring they work as expected.
To run the unit test, complete the following steps:

#. Go to the unit test folder:

   .. code-block:: bash

      cd sidewalk/tests/unit/<test_name>

#. Build the test application by running the following command:

   .. code-block:: bash

      west build --no-sysbuild -b unit_testing

#. Run the test application by running the following command:

   .. code-block:: bash

      ./build/testbinary

Integration testing
===================

Integration tests are designed to ensure that multiple components function together as expected.
The objectives of those tests are as follows:

* Ensuring that various files and functions operate as one module, for example, crypto, timer, and storage.
  Those tests typically use the Sidewalk Platform Abstraction Layer (PAL) API, and have a dedicated Kconfig option that enables or disables the module.
* Validating that the module performs correctly on the :ref:`supported SoCs <hardware_requirements>`.
  This type of testing is occasionally referred to as *unit test on hardware*.

To run the integration test, complete the following steps based on the environment you are using:

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

    .. tab:: Development kits

        #. Go to the integration test folder:

            .. code-block:: bash

                cd sidewalk/tests/integration/<test_name>

        #. Build the test application by running the following command:

            .. code-block:: bash

                west build -b <board_name>

            Use your board name e.g. ``nRF52840dk/nrf52840``, ``nRF55l15dk/nrf54l15/cpuapp``.

        #. Flash the test application by running the following command:

            .. code-block:: bash

                west flash

            Make sure your board is connected to the computer.
            Test output will be displayed on the UART console.

.. _sidewalk_release_notes:

Release notes
#############

See the release notes for the information about specific Sidewalk support releases for nRF Connect SDK.

[v1.14.0] - 02/01/2023
----------------------------
* Used Sidewalk libraries 1.14.0.
* Added ``Experimental symbol SIDEWALK is enabled`` info to the build.
* Added sid_dut for manual and certification tests.
* Added application for power consumption analysis.
* Added new unit and manual tests.
* Updated documentation.
* Added debug and release configuration.
* Optimized power consumption.
* Added the DFU service to the Sidewalk application.
* Added a configuration for the FSK.
* Fixed known issues:

    * KRKNWK-14728: End Node sporadically freezes on LoRa.
    * KRKNWK-14235: When using No Optimization (-O0), the Zephyr assertion fails in Sidewalk init.
* Identified limitations:

    * Dynamic FSK-WAN Profile switching from 2 to 1 is prohibited.
    * User account can have one FSK gateway per location.

[v1.13.0] - 11/04/2022
----------------------------

* Used Sidewalk libraries 1.13 RC30.
* Fixed known issues:

    * KRKNWK-14425: GPIO interrupt configuration issue.
    * KRKNWK-14890: Fatal error on Button 2 in LoRa/FSK template sample due to assertion failure.
* Reduced memory footprint for "BLE only" template variant.
* Support for automatic registration with Sidewalk Frustration Free Networking (FFN).
* Implemented lowpower timers.
* Decreased timer interrupt latency.
* Reduced crypto module memory footprint.
* Added Sidewalk CLI.
* Added support to build with Docker in devcontainer.
* Refactored run test script.
* Added new tests.
* Updated documentation.

[Alpha release] - 07/21/2022
----------------------------

* Used Sidewalk libraries 1.12 RC11.
* Implemented BLE sample.
* Implemented PAL interfaces:

    * Assert
    * BLE
    * Crypto
    * Critical region
    * Delay
    * GPIO
    * Log
    * Storage
    * Temperature
    * Interrupts
    * Timer
    * Uptime
    * SPI
* Created configuration for LORA.
* Integrated tools into sidewalk repository.
* Created documentation.

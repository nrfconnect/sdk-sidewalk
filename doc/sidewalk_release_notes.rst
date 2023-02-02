.. _sidewalk_release_notes:

Release notes
#############

See the release notes for the information about specific Sidewalk support releases for nRF Connect SDK.

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

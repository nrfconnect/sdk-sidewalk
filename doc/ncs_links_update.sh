#!/bin/bash

# Get NCS tag
current_dir=$(pwd)
cd $ZEPHYR_BASE/../nrf;
last_tag=$(git describe --tags --abbrev=0)
last_tag="${last_tag:1}"
cd $current_dir

# Links with NCS tag
links=( ".. _nRF Connect SDK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/index.html"
        ".. _nrf52840 DK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/config_and_build/board_support.html#boards-included-in-sdk-zephyr"
        ".. _nrf5340 DK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/config_and_build/board_support.html#boards-included-in-sdk-zephyr"
        ".. _Getting started with nRF52 Series: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/device_guides/working_with_nrf/nrf52/gs.html"
        ".. _Getting started with nRF53 Series: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/device_guides/nrf53.html"
        ".. _Zephyr toolchain: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/gs_installing.html#install-a-toolchain"
        ".. _nRF Connect SDK Getting started: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/getting_started.html"
        ".. _nRF52840dk_nrf52840: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/zephyr/boards/arm/nrf52840dk_nrf52840/doc/index.html"
        ".. _nrf5340dk_nrf5340: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/zephyr/boards/arm/nrf5340dk_nrf5340/doc/index.html#nrf5340dk-nrf5340"
        ".. _Building and programming an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/getting_started/programming.html#gs-programming"
        ".. _Testing and debugging an application: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/getting_started/testing.html#gs-testing"
        ".. _Bootloader and DFU solutions for NCS: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/app_bootloaders.html"
        ".. _Zephyr SMP Server: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/zephyr/services/device_mgmt/ota.html#smp-server"
        ".. _Zephyr SMP Server sample: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/zephyr/samples/subsys/mgmt/mcumgr/smp_svr/README.html#smp-svr"
        ".. _Zephyr State Machine Framework: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/zephyr/services/smf/index.html"
        ".. _NCS testing applications: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/gs_testing.html"
        ".. _Known issues for the nRF Connect SDK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/<NCS_TAG>/nrf/releases_and_maturity/known_issues.html"
        ".. _Release notes for the nRF Connect SDK: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/releases_and_maturity/releases/release-notes-<NCS_TAG>.html"
)

# Print ncs links with the proper tag
echo -e "..\n\tNote: This file was automatically generated.\n\tPlease modify in script file: $(basename "$0")\n"

for link in "${links[@]}"; do echo "${link/<NCS_TAG>/"$last_tag"}"; done


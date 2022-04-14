#!/bin/bash
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause
#

SIDEWALK_BASE=${ZEPHYR_BASE}/../sidewalk
TWISTER_BIN=${ZEPHYR_BASE}/scripts/twister
HARDWARE_MAP_FILE=${SIDEWALK_BASE}/scripts/ci/map.yml

# Treat unset variables as errors when performing parameter expansion
set -o nounset -o errexit
source ${ZEPHYR_BASE}/../zephyr/zephyr-env.sh

echo "> Build samples"
$TWISTER_BIN --verbose --platform nrf52840dk_nrf52840 --build-only --show-footprint --testcase-root $SIDEWALK_BASE

echo "> Run unit tests"
$TWISTER_BIN --verbose --platform native_posix --coverage  --testcase-root $SIDEWALK_BASE/tests/unit_tests

echo "> Run nRF plaftorm integration tests"
if [ -f "$HARDWARE_MAP_FILE" ]; then
	$TWISTER_BIN --verbose --platform nrf52840dk_nrf52840 --testcase-root $SIDEWALK_BASE/tests/nrf_integration --device-testing --hardware-map $HARDWARE_MAP_FILE
else
	echo "Can't run integration tests. File: $HARDWARE_MAP_FILE does not exist."
	echo "Follow the https://projecttools.nordicsemi.no/confluence/display/KRKNWK/SID2%3A+How+to+run+tests+with+Twister instruction"
	echo "to prepare required map.yml file."
	exit 1
fi

exit 0


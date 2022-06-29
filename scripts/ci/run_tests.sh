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

echo "> Test commit"
git -C $SIDEWALK_BASE log --oneline -1

echo "> Remove previous builds"
rm -rf twister-*

echo "> Generate map file"
$TWISTER_BIN -v --generate-hardware-map $HARDWARE_MAP_FILE 
python3 "$SIDEWALK_BASE/scripts/ci/fill_hardware.py" $HARDWARE_MAP_FILE 

echo "> Build samples"
$TWISTER_BIN --verbose --build-only --show-footprint --testcase-root $SIDEWALK_BASE/samples

echo "> Run unit tests"
$TWISTER_BIN --verbose --platform native_posix --coverage --testcase-root $SIDEWALK_BASE/tests/unit_tests

echo "> Remove files from coverage that are not under test"
lcov -q --remove twister-out/coverage.info '/**/twister-out/*' '/**/mbedtls/*' '/**/test/cmock/*' '/**/zephyr/*' \
-o ${SIDEWALK_BASE}/coverage.info

echo "> Run nRF plaftorm functional tests"
if [ -f "$HARDWARE_MAP_FILE" ]; then
	$TWISTER_BIN --verbose --testcase-root $SIDEWALK_BASE/tests/functional --device-testing --hardware-map $HARDWARE_MAP_FILE
else
	echo "Can't run functional tests. File: $HARDWARE_MAP_FILE does not exist."
	echo "Follow the https://projecttools.nordicsemi.no/confluence/display/KRKNWK/SID2%3A+How+to+run+tests+with+Twister instruction"
	echo "to prepare required map.yml file."
	exit 1
fi

exit 0


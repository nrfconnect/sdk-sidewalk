#!/bin/bash
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
function err_trap () {
	echo "$0: line $1: exit status of last command: $2"
	exit 1
}

set -E
trap 'err_trap ${LINENO} ${$?}' ERR

TWISTER_BIN=${ZEPHYR_BASE}/scripts/twister
if ! [ -e "${TWISTER_BIN}" ]; then
  echo "[ERROR]: ${TWISTER_BIN} does not exists. Did you set ZEPHYR_BASE?"
  exit 1
fi
CURRENT_DIR=$(dirname "$0")
CURRENT_DIR=$(realpath "${CURRENT_DIR}")
SIDEWALK_SDK_DIR=$(realpath "${CURRENT_DIR}/../..")
TESTCASE_ROOT=""

RUN_BUILD=false
RUN_UT=false
RUN_ON_HW=false
USERDEV_CONF_FILE="$CURRENT_DIR/../TargetTestsDevConf/user_devconf.yml"
TWISTER_TAGS=""
TWISTER_PLATFORM=""

LCOV_EXCLUDE=('/**/twister-out/*' '/**/mbedtls/*' '/**/test/cmock/*' '/**/zephyr/*')

usage () {
  echo ""
  echo "Twister test case runner"
	echo "Usage: [-s SIDEWALK_SDK_DIR] [-t TESTCASE_ROOT] [-b] [-u] [-f USERDEV_CONF_FILE] [-a TAG] [-p PLATFORM]"
	echo ""
}

descr () {
	echo "Input parameters:"
	echo "    -s  Path to the SIDEWALK SDK. If not passed, CURRENT_DIR/../.. is used"
	echo "    -t  Overwrite testcase-root to use. If not passed, testcase-root=SIDEWALK_SDK_DIR"
	echo "    -b build without running any test. Use testcase-root SIDEWALK_SDK_DIR/samples"
	echo "    -u run unit tests on native_posix. Use testcase-root SIDEWALK_SDK_DIR/tests/unit_tests"
	echo "    -f run unit tests functional on nRF HW. Use testcase-root SIDEWALK_SDK_DIR/tests/functional"
    echo "    -a run build or tests only for entries with selected tag. It can be passed multiple times, ones per tag. e.g. -a Sidewalk -a Sidewalk_cli"
    echo "    -p run build or tests only for selected platforms. It can be passed multiple times, e.g. -p nrf5340dk_nrf5340_cpuapp"    
	echo ""
}

# Parse opts
while getopts "s::t::f::a::p::ubh" opt
do
	case $opt in
		s)
			SIDEWALK_SDK_DIR=$(realpath "$OPTARG")
			;;
		t)
			TESTCASE_ROOT=$(realpath "$OPTARG")
			;;
		f)
		  RUN_ON_HW=true
		  if [ "${OPTARG}" == "AUTO" ]; then
		    USERDEV_CONF_FILE=AUTO
		  else
			  USERDEV_CONF_FILE=$(realpath "$OPTARG")
			fi
			;;
        a)
            TWISTER_TAGS="${TWISTER_TAGS}--tag ${OPTARG} "
            ;;            
        p)
            TWISTER_PLATFORM="${TWISTER_PLATFORM}--platform ${OPTARG} "
            ;;             
		u)
			RUN_UT=true
			;;
		b)
			RUN_BUILD=true
			;;
		h)
			usage
			descr
			exit 0
			;;
		*)
			usage
			exit 1
			;;
	esac
done
#####################

function run_twister ()
{
    local cmd="$TWISTER_BIN -vvv -O ${CURRENT_DIR}/twister-out $TWISTER_TAGS $TWISTER_PLATFORM $*"
    echo "${cmd}"
    ${cmd} || { return $?; }
    return 0
}

function get_testcase_root ()
{
    local test_root=$1
    if ! [ "${TESTCASE_ROOT}" = "" ]; then
        test_root=${TESTCASE_ROOT}
    fi
    echo "${test_root}"
}

function get_git_log ()
{
    echo -e "\n***********************************"
    echo "[INFO]: Test commit"
    git -C "${SIDEWALK_SDK_DIR}" log --oneline -1
}

function run_build ()
{
    echo -e "\n***********************************\nBUILD\n"
    if [ $RUN_BUILD = false ]; then
        echo "[INFO]: Build test not executed"
        return 0
    fi
    run_twister --build-only --show-footprint -T $(get_testcase_root $(realpath ${SIDEWALK_SDK_DIR}/samples)) || { return $?; }
    mv "${CURRENT_DIR}/twister-out" "${CURRENT_DIR}/twister-out-build"

	python3 ${CURRENT_DIR}/memory_requirements.py --twister-out-dir "${CURRENT_DIR}/twister-out-build" --ncs-dir "${ZEPHYR_BASE}/../" > "${CURRENT_DIR}/twister-out-build/memory_report.rst"
}

function run_ut ()
{
    echo -e "\n***********************************\nUT on POSIX\n"
    if [ $RUN_UT = false ]; then
        echo "[INFO]: Run unit tests not executed"
        return 0
    fi
    run_twister --platform native_posix --platform unit_testing --coverage --enable-ubsan --enable-lsan --enable-asan -T $(get_testcase_root $(realpath ${SIDEWALK_SDK_DIR}/tests/unit_tests))
    echo "[INFO]: Remove files from coverage that are not under test and regenerate html report"
    lcov -q --remove "${CURRENT_DIR}/twister-out/coverage.info" "${LCOV_EXCLUDE[@]}" -o "${CURRENT_DIR}/twister-out/new_coverage.info"
    mv "${CURRENT_DIR}/twister-out/new_coverage.info" "${CURRENT_DIR}/twister-out/coverage.info"
    rm -rf "${CURRENT_DIR}/twister-out/coverage"
    genhtml "${CURRENT_DIR}/twister-out/coverage.info" -o "${CURRENT_DIR}/twister-out/coverage" -q --ignore-errors source --branch-coverage --highlight --legend
    mv "${CURRENT_DIR}/twister-out" "${CURRENT_DIR}/twister-out-posix"
    return $?
}

function run_on_hw ()
{
    echo -e "\n***********************************\nUT on HW\n"
    if [ $RUN_ON_HW = false ]; then
        echo "[INFO]: Run on HW tests not executed"
        return 0
    fi
    if [ "${USERDEV_CONF_FILE}" != "AUTO" ]; then
      if ! [ -e "${USERDEV_CONF_FILE}" ]; then
          echo "[ERROR]: Run on HW not executed because $USERDEV_CONF_FILE does not exists"
          return 1
      fi
    fi
    run_twister -v --generate-hardware-map hardware-map.yaml --persistent-hardware-map
    python3 "$CURRENT_DIR/fill_hardware_map.py" --hardware_map_path hardware-map.yaml  --userdev_conf_path "$USERDEV_CONF_FILE"
    run_twister --platform nrf52840dk_nrf52840 --platform nrf5340dk_nrf5340_cpuapp -T $(get_testcase_root $(realpath ${SIDEWALK_SDK_DIR}/tests/functional)) -T $(get_testcase_root $(realpath ${SIDEWALK_SDK_DIR}/tests/unit_tests)) -T $(get_testcase_root $(realpath ${SIDEWALK_SDK_DIR}/tests/validation)) --device-testing --hardware-map hardware-map.yaml_filled --west-flash="--recover,--erase"
    mv "${CURRENT_DIR}/twister-out" "${CURRENT_DIR}/twister-out-on_HW"
    return $?
}

function main ()
{
    local results=()
    if [ "${CURRENT_DIR}" = "${SIDEWALK_SDK_DIR}" ]; then
        usage
        descr
        exit 0
    fi
    get_git_log
    run_build || { echo "[ERROR]: Build fails"; results+=("run_build"); }
    run_ut || { echo "[ERROR]: Build fails"; results+=("run_ut"); }
    run_on_hw || { echo "[ERROR]: Build fails"; results+=("run_on_hw"); }

    echo -e "\n***********************************"
    echo "STATUS: No of fails: ${#results[@]}, ${results[*]}"
    echo -e "***********************************"
    if [ ${#results[@]} -gt 0 ]; then
        exit 1
    else
        exit 0
    fi
}
####################

main

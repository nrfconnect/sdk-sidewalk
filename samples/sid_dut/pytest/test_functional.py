# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import os
from pathlib import Path
from dotenv import load_dotenv
from twister_harness.twister_harness_config import DeviceConfig, TwisterHarnessConfig
from twister_harness.device.factory import DeviceFactory
from twister_harness.device.device_abstract import DeviceAbstract
from typing import Generator, Type
import time
import logging
import random
import pytest

logger = logging.getLogger(__name__)


load_dotenv(dotenv_path=Path(__file__).parent.joinpath('.env.secrets'))
load_dotenv(dotenv_path=Path(__file__).parent.joinpath('.env.constants'))

MFG_HEX_PATH = os.getenv('MFG_HEX_PATH')

FAST_RESPONSE_TIMEOUT = int(os.getenv('FAST_RESPONSE_TIMEOUT'))
FLASH_MFG_TIMEOUT = int(os.getenv('FLASH_MFG_TIMEOUT'))
DEFAULT_TIMEOUT = int(os.getenv('DEFAULT_TIMEOUT'))


@pytest.fixture(scope='session')
def dut(request: pytest.FixtureRequest) -> Generator[DeviceAbstract, None, None]:
    """Return device instance."""
    twister_harness_config: TwisterHarnessConfig = request.config.twister_harness_config  # type: ignore
    device_config: DeviceConfig = twister_harness_config.devices[0]
    device_type = device_config.type

    device_class: Type[DeviceAbstract] = DeviceFactory.get_device(device_type)

    device = device_class(device_config)

    try:
        device.connect()
        device.generate_command()
        device.initialize_log_files()
        device.run_custom_script(
            ["nrfjprog", "--snr", device_config.id.lstrip('0'), "--recover"], FLASH_MFG_TIMEOUT)
        device.run_custom_script(
            ["nrfjprog", "--snr", device_config.id.lstrip('0'), "--eraseall"], FLASH_MFG_TIMEOUT)
        device.flash_and_run()
        device.connect()
        program_mfg_command = ["nrfjprog", "--program", MFG_HEX_PATH, "--snr",
                               device_config.id.lstrip('0'),  "--sectorerase", "--verify", "-r"]
        logger.debug(program_mfg_command)
        device.run_custom_script(program_mfg_command, FLASH_MFG_TIMEOUT)
        yield device
    except KeyboardInterrupt:
        pass
    finally:  # to make sure we close all running processes after user broke execution
        device.disconnect()
        device.stop()


def wait_for_message(dut: DeviceAbstract, message, timeout=DEFAULT_TIMEOUT, tally=[]):
    time_started = time.time()
    for line in dut.iter_stdout:
        if line:
            logger.debug("#: " + line)
        if message in line:
            logger.info(line)
            logger.info(
                f"detected message \"{message}\" after {time.time() - time_started:.4f}s")
            tally.append(time.time() - time_started)
            return True
        if time.time() > time_started + timeout:
            logger.error(
                f"message {message} has not been found within {timeout}s")
            return False


def wait_for_prompt(dut: DeviceAbstract, prompt='uart:~$', timeout=20):
    time_started = time.time()
    while True:
        dut.write(b'\n')
        for line in dut.iter_stdout:
            if prompt in line:
                logger.debug('Got prompt')
                return True
        if time.time() > time_started + timeout:
            logger.error(f"prompt has not been found within {timeout}s")
            return False


def log_output(dut: DeviceAbstract):
    for line in dut.iter_stdout:
        logger.debug("#: " + line)
        if line == "":
            return


BLE = 1
FSK = 2
LORA = 3


@pytest.mark.parametrize(
    "transport",
    [
        BLE,
        # FSK,
        # LORA,
    ],
)
def test_functional_init_start_stop_deinit_multiple(dut: DeviceAbstract, transport):
    logger.debug('start of bug_test')

    log_output(dut)
    time.sleep(1)
    wait_for_prompt(dut)
    dut.write(f'sid init {transport}\n'.encode('ascii'))
    assert wait_for_message(dut, "sid_init returned 0", FAST_RESPONSE_TIMEOUT)
    log_output(dut)
    dut.write(b'sid start\n')
    assert wait_for_message(dut, "sid_start returned 0", FAST_RESPONSE_TIMEOUT)
    assert wait_for_message(dut, "status changed to")
    assert wait_for_message(dut, "Reg: 0, Time: 0")
    for i in range(20):
        logger.info(f"transport {transport}, iteration {i+1}/20")
        dut.write(b'sid stop\n')
        assert wait_for_message(
            dut, "sid_stop returned 0", FAST_RESPONSE_TIMEOUT)
        assert wait_for_message(dut, "status changed to")
        log_output(dut)
        dut.write(b'sid deinit\n')
        assert wait_for_message(
            dut, "sid_deinit returned 0", FAST_RESPONSE_TIMEOUT)
        log_output(dut)
        dut.write(f'sid init {transport}\n'.encode('ascii'))
        assert wait_for_message(
            dut, "sid_init returned 0", FAST_RESPONSE_TIMEOUT)
        log_output(dut)
        dut.write(b'sid start\n')
        assert wait_for_message(
            dut, "sid_start returned 0", FAST_RESPONSE_TIMEOUT)
        assert wait_for_message(dut, "status changed to")
        assert wait_for_message(dut, "Reg: 0, Time: 0")
        random_sleep = random.random()*2
        logger.info(f"sleeping for {random_sleep}s")
        time.sleep(random_sleep)
        log_output(dut)
    dut.write(b'sid stop\n')
    assert wait_for_message(dut, "sid_stop returned 0", FAST_RESPONSE_TIMEOUT)
    assert wait_for_message(dut, "status changed to")
    log_output(dut)
    dut.write(b'sid deinit\n')
    assert wait_for_message(
        dut, "sid_deinit returned 0", FAST_RESPONSE_TIMEOUT)
    log_output(dut)
    logger.debug('end of bug_test')

# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

"""Host (x86) tests of the west sid provision command, no nRF device needed.

The command reads the mfg_storage partition address from the application build
output, generates the manufacturing HEX file, and programs it with nrfutil, as
described in doc/setting_up_sidewalk_environment/setting_up_sidewalk_prototype.rst.
The build output, the device certificate JSON file and nrfutil are mocked in
test_data; the tests check the HEX file that the nrfutil stub was given.
"""

from __future__ import annotations

import argparse
import os
import shutil
import sys
import tempfile
import unittest

from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent / "test_data"))
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

import mocks  # noqa: E402

from intelhex import IntelHex  # noqa: E402
from sid import Sid  # noqa: E402


class TestSidProvision(unittest.TestCase):
    def setUp(self):
        self.tmp_dir = Path(tempfile.mkdtemp(prefix="sid_provision_test_"))
        self.addCleanup(shutil.rmtree, self.tmp_dir, True)

        self.nrfutil = mocks.NrfutilStub(self.tmp_dir)
        self.connect_devices(["1050000000"])

    def connect_devices(self, serial_numbers: list[str]) -> None:
        for name, value in self.nrfutil.environment(serial_numbers).items():
            previous = os.environ.get(name)
            if previous is None:
                self.addCleanup(os.environ.pop, name, None)
            else:
                self.addCleanup(os.environ.__setitem__, name, previous)
            os.environ[name] = value

    def run_command(self, *args: str) -> None:
        parser = argparse.ArgumentParser(prog="west")
        command = Sid()
        command.do_add_parser(parser.add_subparsers(
            dest="subcommand", required=True))
        command.do_run(parser.parse_args(["sid", *args]), [])

    def assert_mfg_hex(self, offset: int, size: int = mocks.MFG_STORAGE_SIZE) -> None:
        """Check the start address and the provisioning data of the HEX file."""
        self.assertTrue(self.nrfutil.programmed_hex.is_file(),
                        "no HEX file was handed over for programming")
        ihex = IntelHex(str(self.nrfutil.programmed_hex))

        self.assertEqual(
            ihex.minaddr(), offset,
            f"HEX starts at {ihex.minaddr():#x}, expected the mfg_storage "
            f"partition address {offset:#x}")
        self.assertLess(
            ihex.maxaddr(), offset + size,
            f"HEX ends at {ihex.maxaddr():#x}, outside the mfg_storage partition")

        data = ihex.tobinstr()
        for name, value in mocks.certificate_values().items():
            self.assertIn(value, data,
                          f"{name} missing from the manufacturing HEX file")

    def test_provision_sysbuild_build(self):
        """The documented flow: west sid provision -d build -C certificate.json."""
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)

        self.run_command("provision", "-d", str(build_dir),
                         "-C", str(mocks.CERTIFICATE_JSON))

        self.assert_mfg_hex(0x17C000)
        calls = self.nrfutil.calls
        self.assertEqual(calls[0], ["--json", "device", "list"])
        self.assertEqual(calls[-1][:2], ["device", "program"])
        self.assertIn("1050000000", calls[-1])

    def test_provision_single_image_build(self):
        """A --no-sysbuild build directory has no domains.yaml."""
        build_dir = mocks.single_image_build(self.tmp_dir, 0xFF000)

        self.run_command("provision", "-d", str(build_dir),
                         "-C", str(mocks.CERTIFICATE_JSON))

        self.assert_mfg_hex(0xFF000)

    def test_address_is_taken_from_the_build_output(self):
        """The address must follow the build, not a hardcoded default."""
        build_dir = mocks.single_image_build(self.tmp_dir, 0xFC000)

        self.run_command("provision", "-d", str(build_dir),
                         "-C", str(mocks.CERTIFICATE_JSON))

        self.assert_mfg_hex(0xFC000)

    def test_provision_with_explicit_serial_number(self):
        """--serial-number skips device discovery."""
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)

        self.run_command("provision", "-d", str(build_dir),
                         "-C", str(mocks.CERTIFICATE_JSON),
                         "--serial-number", "1050999999")

        self.assert_mfg_hex(0x17C000)
        calls = self.nrfutil.calls
        self.assertEqual(
            len(calls), 1, "nrfutil device list should not be called")
        self.assertEqual(calls[0][:2], ["device", "program"])
        self.assertIn("1050999999", calls[0])

    def test_no_device_connected(self):
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)
        self.connect_devices([])

        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(build_dir),
                             "-C", str(mocks.CERTIFICATE_JSON))

    def test_missing_build_directory(self):
        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(self.tmp_dir / "no_build"),
                             "-C", str(mocks.CERTIFICATE_JSON))

    def test_missing_mfg_storage_partition(self):
        build_dir = self.tmp_dir / "build"
        dts = build_dir / "zephyr" / "zephyr.dts"
        dts.parent.mkdir(parents=True)
        dts.write_text("/dts-v1/;\n\n/ {\n};\n")

        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(build_dir),
                             "-C", str(mocks.CERTIFICATE_JSON))

    def test_missing_certificate_json(self):
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)

        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(build_dir),
                             "-C", str(self.tmp_dir / "missing.json"))

    def test_missing_device_json_arguments(self):
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)

        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(build_dir))

    def test_certificate_json_and_device_json_are_exclusive(self):
        build_dir = mocks.sysbuild_build(self.tmp_dir, 0x17C000)

        with self.assertRaises(SystemExit):
            self.run_command("provision", "-d", str(build_dir),
                             "-C", str(mocks.CERTIFICATE_JSON),
                             "-W", str(mocks.CERTIFICATE_JSON))


if __name__ == "__main__":
    unittest.main()

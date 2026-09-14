# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

"""Mocks for the west sid provision tests: build output, device JSON and nrfutil.

``certificate.json`` is derived from the :file:`certificate_template.json` of the
onboarding tooling, with dummy certificate chains of the sizes that
:file:`tools/provision/provision.py` expects and with a distinct value for every
field that the tests look for in the manufacturing HEX file.
"""

from __future__ import annotations

import base64
import binascii
import json
import os
import stat
import sys

from pathlib import Path

import nrfutil

TEST_DATA_DIR = Path(__file__).resolve().parent
CERTIFICATE_JSON = TEST_DATA_DIR / "certificate.json"
DTS_TEMPLATE = TEST_DATA_DIR / "zephyr.dts.in"
STUB = TEST_DATA_DIR / "nrfutil.py"

MFG_STORAGE_SIZE = 0x1000

APP_DOMAIN = "sid_end_device"


def certificate_values() -> dict[str, bytes]:
    """Provisioning data of certificate.json that the HEX file must contain."""
    certificate = json.loads(CERTIFICATE_JSON.read_text())
    ed25519 = base64.b64decode(certificate["eD25519"])
    p256r1 = base64.b64decode(certificate["p256R1"])
    metadata = certificate["metadata"]
    return {
        "smsn": binascii.unhexlify(metadata["smsn"]),
        "apid": metadata["deviceTypeId"][-4:].encode(),
        "app_server_public_key": binascii.unhexlify(
            certificate["applicationServerPublicKey"]),
        "ed25519_device_pub": ed25519[32:64],
        "ed25519_device_signature": ed25519[64:128],
        "p256r1_device_pub": p256r1[32:96],
        "p256r1_device_signature": p256r1[96:160],
    }


def write_dts(build_dir: Path, offset: int, size: int = MFG_STORAGE_SIZE) -> None:
    """Write a build output with the mfg_storage partition at the given offset."""
    dts = build_dir / "zephyr" / "zephyr.dts"
    dts.parent.mkdir(parents=True, exist_ok=True)
    dts.write_text(
        DTS_TEMPLATE.read_text()
        .replace("@ADDR@", f"{offset:x}")
        .replace("@SIZE@", f"{size:#x}")
    )


def single_image_build(root: Path, offset: int) -> Path:
    """Create a --no-sysbuild build directory, with zephyr.dts directly under it."""
    build_dir = root / "build"
    write_dts(build_dir, offset)
    return build_dir


def sysbuild_build(root: Path, offset: int) -> Path:
    """Create a sysbuild build directory, with the application image in a domain."""
    build_dir = root / "build"
    app_dir = build_dir / APP_DOMAIN
    write_dts(app_dir, offset)
    write_dts(build_dir / "mcuboot", 0xDEAD000)
    (build_dir / "domains.yaml").write_text(
        f"default: {APP_DOMAIN}\n"
        f"build_dir: {build_dir}\n"
        "domains:\n"
        "  - name: mcuboot\n"
        f"    build_dir: {build_dir / 'mcuboot'}\n"
        f"  - name: {APP_DOMAIN}\n"
        f"    build_dir: {app_dir}\n"
        "flash_order:\n"
        "  - mcuboot\n"
        f"  - {APP_DOMAIN}\n"
    )
    return build_dir


class NrfutilStub:
    """The nrfutil stub installed on the PATH, and what it recorded.

    :file:`nrfutil.py` is not executable, so it is installed as a shell script
    that runs it with the Python interpreter of the tests.
    """

    def __init__(self, root: Path):
        self.capture_dir = root / "capture"
        self.capture_dir.mkdir()

        self.bin_dir = root / "bin"
        self.bin_dir.mkdir()
        executable = self.bin_dir / "nrfutil"
        executable.write_text(
            f'#!/bin/sh\nexec "{sys.executable}" "{STUB}" "$@"\n')
        executable.chmod(executable.stat().st_mode |
                         stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

    def environment(self, serial_numbers: list[str]) -> dict[str, str]:
        """Environment that puts the stub on the PATH and lists connected devices."""
        return {
            "PATH": f"{self.bin_dir}{os.pathsep}{os.environ['PATH']}",
            nrfutil.ENV_CAPTURE_DIR: str(self.capture_dir),
            nrfutil.ENV_SERIAL_NUMBERS: ",".join(serial_numbers),
        }

    @property
    def calls(self) -> list[list[str]]:
        """Arguments of every nrfutil call, in order."""
        calls_file = self.capture_dir / nrfutil.CALLS_FILE
        if not calls_file.is_file():
            return []
        return [json.loads(line) for line in calls_file.read_text().splitlines()]

    @property
    def programmed_hex(self) -> Path:
        """The HEX file handed over for programming."""
        return self.capture_dir / nrfutil.PROGRAMMED_HEX_FILE

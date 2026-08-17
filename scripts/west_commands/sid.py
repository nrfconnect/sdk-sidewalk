#!/usr/bin/env python3
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from __future__ import annotations

import json
import re
import subprocess
import sys
import tempfile
import yaml

from pathlib import Path
from west.commands import WestCommand


def resolve_app_build_dir(build_dir: Path) -> Path:
    if (build_dir / "zephyr" / "zephyr.dts").is_file():
        return build_dir

    domains_file = build_dir / "domains.yaml"
    if not domains_file.is_file():
        raise FileNotFoundError(f"no domains.yaml found under {build_dir}")

    data = yaml.safe_load(domains_file.read_text())
    default = data["default"]
    domain_dirs = {d["name"]: Path(d["build_dir"]) for d in data["domains"]}
    if default not in domain_dirs:
        raise ValueError(f'domain "{default}" not found in {domains_file}')

    app_dir = domain_dirs[default].resolve()
    dts = app_dir / "zephyr" / "zephyr.dts"
    if not dts.is_file():
        raise FileNotFoundError(
            f"no zephyr.dts in default domain build dir {app_dir}"
        )
    return app_dir


def parse_mfg_storage_offset(dts_path: Path) -> int:
    text = dts_path.read_text()
    match = re.search(
        r"mfg_storage:\s*partition@[0-9a-fA-F]+\s*\{[^}]*?"
        r"reg\s*=\s*<\s*(0x[0-9a-fA-F]+)\s*(0x[0-9a-fA-F]+)\s*>",
        text,
        re.DOTALL,
    )
    if not match:
        raise ValueError(f"mfg_storage partition not found in {dts_path}")
    return int(match.group(1), 0)


def list_nrfutil_devices() -> list[str]:
    proc = subprocess.run(
        ["nrfutil", "--json", "device", "list"],
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode:
        sys.exit(proc.stderr or proc.stdout or "nrfutil device list failed")

    devices: list[str] = []
    for line in proc.stdout.splitlines():
        if not line.strip():
            continue
        msg = json.loads(line)
        if msg.get("type") == "task_end":
            for dev in msg["data"]["data"]["devices"]:
                if dev["traits"].get("jlink"):
                    devices.append(dev["serialNumber"])
    return devices


def select_device(devices: list[str]) -> str:
    if not devices:
        sys.exit("no connected J-Link devices found")
    if len(devices) == 1:
        return devices[0]
    if not sys.stdin.isatty():
        sys.exit(
            f"{len(devices)} devices connected; specify one with --serial-number "
            "or run from an interactive terminal"
        )

    print(f"Multiple devices connected ({len(devices)}):")
    for idx, serial in enumerate(devices, 1):
        print(f"  {idx}. {serial}")

    prompt = f"Select device (1-{len(devices)}): "
    while True:
        try:
            choice = int(input(prompt))
        except (EOFError, ValueError):
            continue
        if 1 <= choice <= len(devices):
            return devices[choice - 1]


def run_provision_script(
    provision_py: Path,
    offset: int,
    output_hex: Path,
    wireless_device_json: Path | None,
    device_profile_json: Path | None,
    certificate_json: Path | None,
) -> None:
    cmd = [
        sys.executable,
        str(provision_py),
        "nordic",
        "aws",
        "--addr",
        hex(offset),
        "--output_hex",
        str(output_hex),
        "--output_bin",
        "/dev/null",
    ]
    if certificate_json:
        cmd += ["--certificate_json", str(certificate_json)]
    else:
        cmd += [
            "--wireless_device_json",
            str(wireless_device_json),
            "--device_profile_json",
            str(device_profile_json),
        ]

    proc = subprocess.run(cmd, check=False)
    if proc.returncode:
        sys.exit(proc.returncode)


def program_device(hex_path: Path, serial_number: str) -> None:
    proc = subprocess.run(
        [
            "nrfutil",
            "device",
            "program",
            "--options",
            "chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,"
            "reset=RESET_PIN,verify=VERIFY_READ",
            "--firmware",
            str(hex_path),
            "--serial-number",
            serial_number,
        ],
        check=False,
    )
    if proc.returncode:
        sys.exit(proc.returncode)


class Sid(WestCommand):
    def __init__(self):
        super().__init__(
            name="sid",
            help="Sidewalk utility commands",
            description="Sidewalk utility commands.",
        )

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description,
        )
        subparsers = parser.add_subparsers(dest="command", required=True)

        provision = subparsers.add_parser(
            "provision",
            help="provision mfg_storage partition on a connected board",
        )
        provision.add_argument(
            "-d",
            metavar="DIR",
            dest="build_dir",
            type=Path,
            default=Path.cwd() / "build",
            help='build directory (default: "$PWD/build")',
        )
        provision.add_argument(
            "-W",
            "--wireless-device-json",
            type=Path,
            dest="wireless_device_json",
        )
        provision.add_argument(
            "-D",
            "--device-profile-json",
            type=Path,
            dest="device_profile_json",
        )
        provision.add_argument(
            "-C",
            "--certificate-json",
            type=Path,
            dest="certificate_json",
        )
        provision.add_argument(
            "--serial-number",
            help="J-Link serial number (required when multiple boards are connected)",
        )

        return parser

    def do_run(self, args, unknown_args):
        cert = args.certificate_json
        wireless = args.wireless_device_json
        profile = args.device_profile_json

        if cert and (wireless or profile):
            sys.exit("use either -C/--certificate-json or -W/-D, not both")
        if cert:
            if not cert.is_file():
                sys.exit(f"certificate json not found: {cert}")
        elif wireless and profile:
            if not wireless.is_file():
                sys.exit(f"wireless device json not found: {wireless}")
            if not profile.is_file():
                sys.exit(f"device profile json not found: {profile}")
        else:
            sys.exit(
                "provide -W/--wireless-device-json and -D/--device-profile-json, "
                "or -C/--certificate-json"
            )

        build_dir = args.build_dir.resolve()
        if not build_dir.is_dir():
            sys.exit(f"build directory not found: {build_dir}")

        try:
            app_build_dir = resolve_app_build_dir(build_dir)
            offset = parse_mfg_storage_offset(app_build_dir / "zephyr" / "zephyr.dts")
        except (FileNotFoundError, ValueError) as exc:
            sys.exit(str(exc))

        sidewalk_root = Path(__file__).resolve().parents[2]
        provision_py = sidewalk_root / "tools" / "provision" / "provision.py"
        if not provision_py.is_file():
            sys.exit(f"provision script not found: {provision_py}")

        with tempfile.NamedTemporaryFile(suffix=".hex", delete=False) as tmp:
            hex_path = Path(tmp.name)

            try:
                run_provision_script(
                    provision_py,
                    offset,
                    hex_path,
                    wireless,
                    profile,
                    cert,
                )

                serial = args.serial_number or select_device(list_nrfutil_devices())
                program_device(hex_path, serial)
            finally:
                hex_path.unlink(missing_ok=True)

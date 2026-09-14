# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

"""nrfutil stub used instead of the real tool, so that no nRF device is needed.

``device list`` reports the J-Link serial numbers given in
SID_TEST_NRFUTIL_SERIALS, ``device program`` keeps the HEX file it was given.
Both write their arguments to SID_TEST_NRFUTIL_CAPTURE.
"""

import json
import os
import shutil
import sys

from pathlib import Path

CALLS_FILE = "calls.json"
PROGRAMMED_HEX_FILE = "programmed.hex"

ENV_CAPTURE_DIR = "SID_TEST_NRFUTIL_CAPTURE"
ENV_SERIAL_NUMBERS = "SID_TEST_NRFUTIL_SERIALS"


def main(argv: list) -> None:
    capture_dir = Path(os.environ[ENV_CAPTURE_DIR])
    with (capture_dir / CALLS_FILE).open("a") as calls:
        calls.write(json.dumps(argv) + "\n")

    if "list" in argv:
        serials = [s for s in os.environ[ENV_SERIAL_NUMBERS].split(",") if s]
        devices = [{"serialNumber": s, "traits": {"jlink": True}}
                   for s in serials]
        # A device without the J-Link trait must be ignored by the command.
        devices.append({"serialNumber": "usb-only",
                       "traits": {"jlink": False}})
        print(json.dumps({"type": "task_begin", "data": {}}))
        print("")
        print(json.dumps(
            {"type": "task_end", "data": {"data": {"devices": devices}}}))
    elif "program" in argv:
        firmware = argv[argv.index("--firmware") + 1]
        shutil.copyfile(firmware, capture_dir / PROGRAMMED_HEX_FILE)
    else:
        sys.exit(f"nrfutil stub: unexpected arguments: {argv}")


if __name__ == "__main__":
    main(sys.argv[1:])

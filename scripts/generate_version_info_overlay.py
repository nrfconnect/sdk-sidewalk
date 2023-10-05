# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import subprocess
from pathlib import Path
import os
import re

assert False, "I want to test something"

def subtext(text, regex, if_none):
    exp = re.search(regex, text)
    return if_none if exp is None else exp.group(1)


git_describe_cmd = subprocess.run(["git", "describe"], cwd=Path(
    os.path.normpath(__file__)).parents[2] / "sidewalk", capture_output=True)
if git_describe_cmd.returncode != 0:
    version = "unknown"

version = git_describe_cmd.stdout.decode("utf-8")
version_numbers = subtext(version, r"v(\d+\.\d+\.\d+)", "0.0.0") + \
    "+" + subtext(version, r"v[\d\.]+-(\d+)", "0")
print(f"CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION=\"{version_numbers}\"")

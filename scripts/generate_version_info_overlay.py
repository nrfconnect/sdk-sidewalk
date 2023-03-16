# Copyright (c) 2023 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

try:
    import git
except ImportError as e:
    raise Exception(
        "GitPython module not found! install packages from `sidewalk/requirements.txt`")
    exit(-1)

from pathlib import Path
import os
import re


def subtext(text, regex, if_none):
    exp = re.search(regex, text)
    return if_none if exp is None else exp.group(1)


version = git.repo.Repo(Path(os.path.normpath(
    __file__)).parents[2] / "sidewalk").git.describe()
version_numbers = subtext(version, "v(\d+\.\d+\.\d+)", "0.0.0") + \
    "+" + subtext(version, "v[\d\.]+-(\d+)", "0")
print(f"CONFIG_MCUBOOT_IMAGE_VERSION=\"{version_numbers}\"")

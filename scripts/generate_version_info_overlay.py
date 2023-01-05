import git
from pathlib import Path
import os
import re

def subtext(text, regex, if_none):
    exp = re.search(regex, text)
    return if_none if exp is None else exp.group(1)

version = git.repo.Repo(Path(os.path.normpath(__file__)).parents[2] / "sidewalk").git.describe()
version_numbers = subtext(version, "v(\d+\.\d+\.\d+)", "0.0.0") + \
    "+" + subtext(version, "v[\d\.]+-(\d+)", "0")
print(f"CONFIG_MCUBOOT_IMAGE_VERSION=\"{version_numbers}\"")

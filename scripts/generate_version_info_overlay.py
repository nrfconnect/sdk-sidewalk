import git
from pathlib import Path
import os

version = git.repo.Repo(Path(os.path.normpath(__file__)).parents[2] / "sidewalk").git.describe()
version_numbers = "+".join(version[1:].split("-")[:-1])
print(f"CONFIG_MCUBOOT_IMAGE_VERSION=\"{version_numbers}\"")

# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

from ruamel.yaml import YAML
import argparse

parser = argparse.ArgumentParser(
    description='Modify revision of NRF in west.yml')

parser.add_argument('manifest_path', help="path to west.yml to modify")
parser.add_argument('-r', '--revision', default="main",
                    help="git hash, branch name or tag")

args = parser.parse_args()

yml = YAML(typ='rt')
yml.indent(mapping=2, sequence=4, offset=2)

with open(args.manifest_path, "r") as f:
    manifest = yml.load(f)

nrf = filter(lambda a: a["name"] == "nrf", manifest["manifest"]["projects"])
# I expect that there will be 1 such element, if not exeption will be thrown by next()
next(nrf)["revision"] = args.revision

with open(args.manifest_path, "w") as f:
    yml.dump(manifest, f)

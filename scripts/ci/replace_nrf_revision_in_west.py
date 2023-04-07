# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import yaml
import sys

manifest = dict()
with open(sys.argv[1], "r") as f:
    manifest = yaml.safe_load(f)

nrf = filter(lambda a: a["name"] == "nrf", manifest["manifest"]["projects"])
# I expect that there will be 1 such element, if not exeption will be thrown by next()
next(nrf)["revision"] = "main"

with open(sys.argv[1], "w") as f:
    yaml.dump(manifest, f)

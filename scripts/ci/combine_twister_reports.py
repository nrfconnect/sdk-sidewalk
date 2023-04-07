# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import json
import sys

if (len(sys.argv) <= 3):
    print("not enough arguments")
    exit(0)

*files_to_combine, output_file = sys.argv[1:]

combined = {"testsuites": []}

for file in files_to_combine:
    try:
        with open(file, "r") as f:
            report = json.load(f)
            combined["testsuites"].extend(report.get("testsuites", []))
    except FileNotFoundError:
        print(f"Failed to open file {file}")

with open(output_file, "w") as f:
    json.dump(combined, f)

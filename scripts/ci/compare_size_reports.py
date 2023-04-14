# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import argparse
import json
import pathlib


def get_arguments():
    parser = argparse.ArgumentParser(
        prog="Compare twister reports for build size differences")
    parser.add_argument("-o", "--old", required=True, type=str)
    parser.add_argument("-n", "--new", required=True, type=str)
    parser.add_argument("--md_output", action='store_true')
    return parser


def convert_unit(value) -> str:
    units = ["B", "KB", "MB", "GB"]
    negative = False
    if value < 0:
        negative = True
        value = -1 * value
    unit_index = 0
    current_value = value
    while (current_value > 1024):
        current_value = current_value / 1024
        unit_index = unit_index+1
    if negative:
        current_value = -1 * current_value
    unit = units[unit_index]
    return f"{current_value:.2f} {unit}"


def print_ouptput(options, diff_result):
    if options.md_output:
        print("||RAM||| ROM|||")
        print("|---|---|---|---|---|---|---|")
        print("| Sample | diff | used | total | diff | used | total |")

        for key, element in diff_result.items():
            diff_ram = element.get("new_used_ram", 0) - \
                element.get("old_used_ram", 0)
            diff_rom = element.get("new_used_rom", 0) - \
                element.get("old_used_rom", 0)

            diff_ram = convert_unit(diff_ram)
            used_ram = convert_unit(element.get("new_used_ram", None))
            avaliable_ram = convert_unit(element.get("available_ram", None))
            diff_rom = convert_unit(diff_rom)
            used_rom = convert_unit(element.get("new_used_rom", None))
            avaliable_rom = convert_unit(element.get("available_rom", None))
            print(
                f"|{key}|{diff_ram}|{used_ram}|{avaliable_ram}|{diff_rom}|{used_rom}|{avaliable_rom}|")
    else:
        print(diff_result)


def main():
    options = get_arguments().parse_args()

    old_report = {}
    new_report = {}

    with open(pathlib.Path(options.old)) as f:
        old_report = json.load(f)

    with open(pathlib.Path(options.new)) as f:
        new_report = json.load(f)

    diff_result = {}

    for element in new_report.get("testsuites", dict()):
        key = element.get("platform", "") + ":" + \
            element.get("name", "").split("/")[-1]
        if diff_result.get(key, None) is None:
            diff_result[key] = {}
        diff_result[key]['new_used_ram'] = element.get("used_ram", 0)
        diff_result[key]['new_used_rom'] = element.get("used_rom", 0)
        diff_result[key]['available_ram'] = element.get("available_ram", 0)
        diff_result[key]['available_rom'] = element.get("available_rom", 0)

    for element in old_report.get("testsuites", dict()):
        key = element.get("platform", "") + ":" + \
            element.get("name", "").split("/")[-1]
        if diff_result.get(key, None) is None:
            diff_result[key] = {}
        diff_result[key]["old_used_ram"] = element.get("used_ram", 0)
        diff_result[key]["old_used_rom"] = element.get("used_rom", 0)

    print_ouptput(options, diff_result)


if __name__ == "__main__":
    main()

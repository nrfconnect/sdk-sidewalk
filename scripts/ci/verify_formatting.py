# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import yaml
import argparse
import logging
from pathlib import Path
import subprocess
import re
logger = logging.getLogger(__name__)


def argument_parser():
    """

    :return:
    """
    parser_ = argparse.ArgumentParser()
    parser_.add_argument("-c", "--config", required=True,
                         help="YAML configuration file")
    parser_.add_argument("-d", "--debug", action="store_true", default=False, required=False,
                         help="Enable debug mode, more logs.")
    action = parser_.add_mutually_exclusive_group(required=True)
    action.add_argument("-s", "--scan-root", nargs=1,
                        help="Check all files in directory")
    action.add_argument("-f", "--files", nargs='+', help="files to check")
    return parser_


class Configuration:
    """
    Load and validate configuration for the verify_license
    """

    def __init__(self, cfg: dict):
        """
        Create configuration object.

        :param cfg_: configuration
        :return: Configuration or exception
        :exceptions: KeyError, ValueError
        """

        self._config = cfg


def split_files_for_languages(files):
    return {
        "source": [
            f for f in files if f[-2:] == ".h" or f[-2:] == ".c"
        ],
        "python": [
            f for f in files if f[-3:] == ".py"
        ]
    }


def get_files_from_git() -> dict():
    git_files_run = subprocess.run(
        "git ls-files".split(" "), capture_output=True)

    git_files = git_files_run.stdout.decode("utf8").split("\n")

    return split_files_for_languages(git_files)


def filter_files(files, filters):
    result = []
    for file in files:
        include = True
        for regexp in filters:
            if re.search(regexp, str(file)):
                include = False
        if include:
            result.append(file)
    return result


if __name__ == "__main__":
    parser = argument_parser()
    args = parser.parse_args()

    loggerFormat = "[%(levelname)-7s] %(asctime)s: %(message)s"
    loggerFormatter = logging.Formatter(loggerFormat)

    if args.debug:
        loggerLevel = logging.DEBUG
    else:
        loggerLevel = logging.INFO
    logging.basicConfig(format=loggerFormat, level=loggerLevel)

    with open(Path(args.config).absolute().resolve(), "r") as cfg_file:
        cfg = Configuration(yaml.safe_load(cfg_file))
    if args.files is None:
        files = get_files_from_git()
    else:
        files = split_files_for_languages(args.files)

    C_files_filtered = filter_files(
        files.get("source"), cfg._config["ignored_files_regexp"])

    py_filtered = filter_files(
        files.get("python"), cfg._config["ignored_files_regexp"])

    top_level_cmd = subprocess.run(
        "git rev-parse --show-toplevel".split(" "), capture_output=True)
    top_level = Path(top_level_cmd.stdout.decode("utf8").strip())
    uncrustify_config = Path(top_level).joinpath(cfg._config["uncrustify_cfg"])

    logger.debug(f"checking {len(C_files_filtered)} C files")
    for file in C_files_filtered:
        logger.debug(file)

    print_diff = False
    if len(C_files_filtered) > 0:
        filtered_files_str = " ".join([str(f) for f in C_files_filtered])
        uncrustify_check = subprocess.run(
            f"uncrustify -c {uncrustify_config} --check {filtered_files_str}".split(" "), cwd=top_level, capture_output=True)

        if uncrustify_check.returncode != 0:
            subprocess.run(
                f"uncrustify -c {uncrustify_config} --replace --no-backup {filtered_files_str}".split(" "), cwd=top_level, capture_output=True)
            print_diff = True

    if len(py_filtered) > 0:
        logger.debug(f"checking {len(py_filtered)} Python files")
        for file in py_filtered:
            logger.debug(file)

        filtered_py_files_str = " ".join([str(f) for f in py_filtered])
        autopep_check = subprocess.run(
            f"autopep8 -i --exit-code {filtered_py_files_str}".split(" "), cwd=top_level, capture_output=True)
        if autopep_check.returncode != 0:
            print_diff = True

    if print_diff:
        changes = subprocess.run(
            f"git diff".split(" "), cwd=top_level, capture_output=True)
        logger.error(changes.stdout.decode("utf-8"))
        exit(1)

    exit(0)

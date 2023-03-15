"""Verify existence of license file headers in files according to yaml configuration file."""
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
import logging
import yaml
from typing import List, Union
from pathlib import Path
import subprocess
import re
import argparse
import sys
MIN_PYTHON = (3, 10)
assert sys.version_info >= MIN_PYTHON, f"requires Python {'.'.join([str(n) for n in MIN_PYTHON])} or newer"


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
        self._validate_ignore_files(or_default=[])
        self._validate_supported_files(or_default=[r"\..*"])
        self._validate_header_size(or_default=30)
        self._validate_licenses()

    @property
    def ignore_files(self) -> set:
        """get list of ignored files

        Returns:
            list: list of regular expressions that describe paths to ignored files
        """
        return set(self._config["ignore_license_in_files"])

    @property
    def supported_file_extensions(self) -> set:
        """get list of supported files

        Returns:
            list: list of regular expressions that describe paths to supported files
        """
        return set(self._config["supported_file_extensions"])

    @property
    def header_lines_limit(self) -> int:
        """get number of lines to read from top of the file

        Returns:
            int: number of lines where we expect to see license
        """
        return self._config["license_header_size"]

    @property
    def licenses(self) -> dict:
        """Get configured licenses."""
        return self._config["licenses"]

    @staticmethod
    def _validate_regular_expression_str(expression: str) -> bool:
        try:
            re.compile(expression)
        except re.error:
            return False
        return True

    def _validate_ignore_files(self, or_default):
        """validate ignore files field

        Args:
            or_default (list): default value if ignore files wa not found

        Raises:
            ValueError: if the passed value is not valid
        """
        if "ignore_license_in_files" not in self._config:
            self._config["ignore_license_in_files"] = or_default
        if not isinstance(self._config["ignore_license_in_files"], list):
            raise ValueError(
                "ignore_license_in_files must be array of strings")
        for element in self._config["ignore_license_in_files"]:
            if not isinstance(element, str):
                raise ValueError(
                    "ignore_license_in_files, can contain only array of strings")
            if not self._validate_regular_expression_str(element):
                raise ValueError(
                    f"ignore_license_in_files = {element} is not valid regular expression")

    def _validate_supported_files(self, or_default):
        """validate supported files field

        Args:
            or_default (list): default value if ignore files wa not found

        Raises:
            ValueError: if the passed value is not valid
        """
        if "supported_file_extensions" not in self._config:
            self._config["supported_file_extensions"] = or_default

        if not isinstance(self._config["supported_file_extensions"], list):
            raise ValueError(
                "supported_file_extensions must be array of strings")

        for element in self._config["supported_file_extensions"]:
            if not isinstance(element, str):
                raise ValueError(
                    "supported_file_extensions, can contain only array of strings")
            if not self._validate_regular_expression_str(element):
                raise ValueError(
                    f"supported_file_extensions = {element} is not valid regular expression")

    def _validate_header_size(self, or_default):
        """validate supported files field

        Args:
            or_default (int): default number of lines read from top of file where license is expected

        Raises:
            ValueError: if the passed value is not valid
        """
        if "license_header_size" not in self._config:
            self._config["license_header_size"] = or_default
        if not isinstance(self._config["license_header_size"], int):
            raise ValueError("license_header_size, must be a number")
        if self._config["license_header_size"] < 0:
            raise ValueError("license_header_size, must be greater than 0")

    def _validate_licenses(self):
        """validate licenses field

        Raises:
            KeyError: if none valid license is present in the configuration
            ValueError: if the passed value is not valid
        """
        if "licenses" not in self._config:
            raise KeyError("licences missing")
        if not isinstance(self._config["licenses"], list):
            raise ValueError(
                "licenses has to contain array of license objects")

        for license_ in self._config["licenses"]:
            if (
                "spdx" not in license_
                or "copyright_regexp" not in license_
                or "search_license_txt" not in license_
                or "file_regexp" not in license_
            ):
                raise KeyError("Missing required element of license field")

            if not isinstance(license_["copyright_regexp"], str):
                raise ValueError(
                    "copyright_regexp, has to be regular expression string")

            if license_["spdx"] is None and license_["search_license_txt"] is None:
                raise ValueError(
                    "specify expected spdx, or provide license string to search in file")

            if not isinstance(license_["spdx"], (str, type(None))):
                raise ValueError("spdx can only be string or null")

            if not isinstance(license_["search_license_txt"], (list, type(None))):
                raise ValueError(
                    "search_license_txt can only be array of strings, or null")

            if not isinstance(license_["file_regexp"], list):
                raise ValueError(
                    "file_regexp must be array of regular expressions")

            for element in license_["file_regexp"]:
                if not self._validate_regular_expression_str(element):
                    raise ValueError(
                        f"file_regexp = {element} is not valid regular expression")


class FileListManager:
    """
    Load list of files to process, and provide filter functionality
    """

    def __init__(self, configuration: Configuration, files: list):
        """Create FileListManager object

        Args:
            configuration (Configuration): configuration object that contains rules for filtering
            files (set): set of file paths 
        """
        self.ignore_files = configuration.ignore_files
        self.supported_files = configuration.supported_file_extensions
        self.files = [str(Path(x).absolute()) for x in files]
        self._remove_ignored_files()
        self._remove_not_supported_files()

    @staticmethod
    def _file_match_any_regexp(file: str, regexp: set) -> bool:
        for expression in regexp:
            if re.search(expression, file):
                return True
        return False

    def _remove_ignored_files(self):
        new_file_set = set(filter(lambda file: not self._file_match_any_regexp(
            file, self.ignore_files), self.files))
        self.files = new_file_set

    def _remove_not_supported_files(self):
        new_file_set = set(filter(lambda file: self._file_match_any_regexp(
            file, self.supported_files), self.files))
        self.files = new_file_set

    @property
    def filtered_files(self) -> set:
        """Get set of files that should be processed

        Returns:
            set: files for processing
        """
        return set(self.files)


class LicenseVerificator:
    """
    Verifies if the header contains expected license
    """

    def __init__(self, configuration: Configuration, file_path: Union[str, Path]):
        """Create LicenseVerificator object

        Args:
            configuration (Configuration): Instance of Configuration 
        """
        self.licenses = configuration.licenses
        self.read_lines = configuration.header_lines_limit
        self.file_path = str(file_path)

    @staticmethod
    def _file_match_any_regexp(file: str, regexp: set) -> bool:
        for expression in regexp:
            if re.search(expression, file):
                return True
        return False

    def _find_expected_license(self, path: Union[str, Path]):
        for lic in self.licenses:
            if self._file_match_any_regexp(path, lic["file_regexp"]):
                return lic
        return None

    def _read_header(self, file_path: Union[str, Path]):
        lines = []
        with open(file_path, "r") as f:
            for idx, line in enumerate(f):
                if idx == self.read_lines:
                    break
                lines.append(line)
        return lines

    @staticmethod
    def find_license_txt(license_txt: list, file_header: str) -> int:
        """Checks the header of file for license txt

        Args:
            license_txt (list): expected license text as set of regular expressions.
                                All of those expressions have to be found
            file_header (str): file header in single str object

        Returns:
            int: if any of the license_txt elements could not be found in file_header, return 1, otherwise 0
        """
        for txt in license_txt:
            normalized_header = re.sub(
                r"[^A-Za-z0-9().,\s]", " ", "".join(file_header.split("\n")))
            normalized_header = re.sub(r"\s+", " ", normalized_header)
            if not re.search(txt, normalized_header, re.IGNORECASE):
                logger.error(f"\tLicense text: {txt} could not be found")
                return 1
        return 0

    @staticmethod
    def find_spdx(expect_spdx: str, file_header: str) -> int:
        """Check if spdx is in file, and if it is the same as expected

        Args:
            expect_spdx (str): expected spdx license
            file_header (str): file header in single str object

        Returns:
            int: return 1 if spdx was not found, or it is not the same as expected, otherwise 0
        """
        spdx = re.search(r"SPDX-License-Identifier:\s+([^\s]*)", file_header)
        if spdx:
            spdx = spdx.groups()[0]
        if spdx != expect_spdx:
            logger.error(
                f"\tInvalid spdx found {spdx}, expected {expect_spdx}")
            return 1
        return 0

    @staticmethod
    def find_copyright(copyright_regexp: str, file_header: str) -> int:
        """Check if copyright message is in file header

        Args:
            copyright_regexp (str): regular expression for copyright message
            file_header (str): file header in single str object

        Returns:
            int: return 1 if the copyright regular expression could not be matched with file header, otherwise 0
        """
        if not re.search(copyright_regexp, file_header):
            logger.error(f"\tMissing or invalid Copyright")
            return 1
        return 0

    def validate_file_header(self, header: List[str], expected_license: dict) -> int:
        """Execute checks on the file header to determine, if expected license is present in file header

        Args:
            header (List[str]): fist N lines of file
            expected_license (dict): license object that describe expected license of the file

        Returns:
            int: 0 if license is correct, positive number indicates number of detected violations
        """
        return_value = 0
        file_header = "".join(header)

        return_value += self.find_copyright(
            expected_license["copyright_regexp"], file_header)
        return_value += self.find_spdx(
            expected_license["spdx"], file_header) if expected_license["spdx"] else 0
        return_value += (
            self.find_license_txt(
                expected_license["search_license_txt"], file_header)
            if expected_license["search_license_txt"]
            else 0
        )
        if return_value == 0:
            logger.debug("Ok")
        return return_value

    def check(self) -> int:
        """Reads file and checks if its header contains correct license

        Returns:
            int: 0 if license is correct, positive number indicates number of detected violations
        """
        expected_license = self._find_expected_license(self.file_path)
        if expected_license is None:
            logger.warning(
                f"File {self.file_path} is not covered by any license in the configuration!")
        file_header = self._read_header(self.file_path)
        logger.debug(f"checking file {self.file_path}")
        ret = self.validate_file_header(file_header, expected_license)
        if ret > 0:
            logger.error(f"Failed check on file: {self.file_path}")

        return ret


def argument_parser():
    """

    :return:
    """
    parser_ = argparse.ArgumentParser()
    parser_.add_argument("-c", "--config", required=True,
                         help="YAML configuration file")
    parser_.add_argument("-d", "--debug", action="store_true", default=False, required=False,
                         help="Enable debug mode, more logs.")
    source_of_files_to_scan = parser_.add_mutually_exclusive_group(
        required=True)
    source_of_files_to_scan.add_argument(
        "-g", "--git-diff", action="store_true", help="check only staged files")
    source_of_files_to_scan.add_argument(
        "-f", "--files", nargs="+", help="Check only selected files")
    source_of_files_to_scan.add_argument(
        "-s", "--scan-root", nargs=1, help="Check all files in repository")
    return parser_


logger = logging.getLogger(__name__)

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

    return_code = 0
    if args.scan_root:
        files = [
            file.absolute().resolve() for file in Path(args.scan_root[0]).rglob("*.*")
        ]
    elif args.git_diff:
        # get root folder of git repo
        out = subprocess.run(
            ["git", "rev-parse", "--show-cdup"], capture_output=True)
        repo_root_path = (
            Path(out.stdout.decode("utf-8").split("\n")
                 [0]).absolute().resolve()
        )
        # get list of staged files
        git_cmd_args = ["git", "diff", "--name-only",
                        "--staged", "--diff-filter=AMR"]
        git_output = subprocess.run(
            git_cmd_args, capture_output=True, cwd=repo_root_path
        )
        modified_files = git_output.stdout.decode("utf-8").split("\n")

        files = [repo_root_path / x for x in modified_files]
    elif args.files:
        files = [Path(file).absolute().resolve() for file in args.files]

    error_count = 0
    checked_files = 0
    for file in FileListManager(cfg, files).filtered_files:
        error_count += LicenseVerificator(cfg, file).check()
        checked_files += 1

    logger.info(f"Checked {checked_files} files:")
    if error_count > 0:
        logger.error(f"END WITH {error_count} ERRORS")
        logger.error(
            f"Check if new files are correctly represented in sidewalk/ci/license.yml")
    else:
        logger.info("END WITH NO ERRORS")
    exit(error_count)

#!/usr/bin/env python3
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import argparse
from collections import namedtuple
from pathlib import Path
import subprocess
from tabulate import tabulate
from typing import Generator, Optional
import yaml


class Sample:
    def __init__(self, relative_dir: str, test_name: str, title: str):
        self.relative_dir = relative_dir
        self.test_name = test_name
        self.title = title

    def full_test_name(self) -> str:
        """
        Return twister test name of the sample.

        The test name consists of the relative path to the sample followed by
        a common prefix for all tests defined in the sample.yaml file of the
        sample.
        """
        return f'{self.relative_dir}/{self.test_name}'


class SampleVariant:
    def __init__(self, sample: Sample, variant: str, title: str):
        self.sample = sample
        self.variant = variant
        self.title = title

    def full_test_name(self) -> str:
        """
        Return twister test name of the sample variant.

        The test name consists of the relative path to the sample followed by
        a test name defined in the sample.yaml file of the sample. It can be
        be specified as twister's '--test' argument.
        """
        return self.sample.full_test_name() + self.variant

    def full_title(self) -> str:
        """
        Return human-readable name of the sample variant.
        """
        return f'{self.sample.title} ({self.title})'


SampleVariantStats = namedtuple('SampleVariantStats', [
    'sample',
    'mcuboot_rom',
    'app_rom',
    'settings',
    'total_rom',
    'total_ram'
])


BOARDS = {
    'nrf52840dk_nrf52840': '`nRF52840 DK`_',
    'nrf5340dk_nrf5340_cpuapp': '`nRF5340 DK`_',
}


SAMPLES = [
    Sample('sid_end_device', 'sample.sidewalk.demo',
           ':ref:`Sensor monitoring <sidewalk_demo>`'),
    Sample('sid_end_device', 'sample.sidewalk.hello',
           ':ref:`End device Bluetooth LE and sub-GHz <sidewalk_hello>`'),
    Sample('sid_end_device', 'sample.sidewalk.hello.ble_only',
           ':ref:`End device Bluetooth LE <sidewalk_hello>`'),
]

VARIANTS = {
    '': 'Debug',
    '.release': 'Release',
}


def removeprefix(text: str, prefix: str) -> str:
    return text[len(prefix):] if text.startswith(prefix) else text


class PartitionParser:
    def __init__(self, partitions_yml_path: Path):
        with open(partitions_yml_path) as partitions_yml:
            self.partitions = yaml.safe_load(partitions_yml)

    def region_size(self, region_name) -> int:
        """
        Return size of a region defined in partitions.yml file in bytes.
        """
        return int(self.partitions.get(region_name, {}).get('size', 0))

    def region_size_kb(self, *region_name) -> int:
        """
        Return size of one or more regions defined in partitions.yml file in kBs.
        """
        return round(sum(self.region_size(rn) for rn in region_name)/1024)


class ElfSizeParser:
    def __init__(self, elf_path: Path):
        sizes = subprocess.check_output(
            ['arm-zephyr-eabi-size', '-B', elf_path])
        sizes = sizes.decode('ascii').splitlines()
        self.sizes = dict(zip(sizes[0].split(), sizes[1].split()))

    def region_size(self, region_name: str) -> int:
        """
        Return size of a region returned by arm-zephyr-eabi-size in bytes.
        """
        return int(self.sizes[region_name])

    def region_size_kb(self, *region_name: str) -> int:
        """
        Return size of one or more regions returned by arm-zephyr-eabi-size in kBs.
        """
        return round(sum(self.region_size(rn) for rn in region_name)/1024)


class ReportGenerator:

    HEADERS = ['Sample', 'MCUboot ROM', 'Application ROM',
               'Sidewalk Settings', 'Total ROM', 'Total RAM']

    def __init__(self):
        print()
        print('.. tabs::')

    def add_board(self, board: str) -> None:
        """
        Generate a new RST tab for holding results for a given board.
        """
        self.variants = []
        tab_title = removeprefix(BOARDS[board], ':ref:`').split('<')[0].strip()
        print()
        print(f'   .. tab:: {tab_title}')
        print()
        print(
            f'      The following table lists memory requirements for samples running on the {BOARDS[board]}.')

    def add_variant(self, stats: SampleVariantStats) -> None:
        """
        Add sample variant statistics to the internal buffer.
        """
        self.variants += [(stats.sample, stats.mcuboot_rom, stats.app_rom,
                           stats.settings, stats.total_rom, stats.total_ram)]

    def flush_variants(self) -> None:
        """
        Consume the buffer of sample variant statistics and generate an RST table.
        """
        print()
        for line in tabulate(self.variants, ReportGenerator.HEADERS, tablefmt='grid').splitlines():
            print(f'      {line}')


def select_sample_variant(test_name: str) -> Optional[SampleVariant]:
    """
    Take test name returned by 'twister --list-tests' and turn it into a sample variant.
    """
    for sample in SAMPLES:
        if not test_name.startswith(sample.test_name):
            continue
        variant = removeprefix(test_name, sample.test_name)
        if variant in VARIANTS:
            return SampleVariant(sample, variant, VARIANTS[variant])
    return None


def select_sample_variants(twister_path: str, twister_out_dir: str, ncs_dir: str) -> Generator[SampleVariant, None, None]:
    """
    Detect available sample variants and return relevant ones.
    """
    cmd = [
        twister_path,
        '--testsuite-root', Path(ncs_dir) / 'sidewalk' / 'samples',
        '--outdir', twister_out_dir,
        '--no-clean',
        '--integration',
        '--list-tests'
    ]

    for test in subprocess.check_output(cmd).decode('ascii').splitlines():
        if variant := select_sample_variant(removeprefix(test, ' - ').strip()):
            yield variant


def build_report(twister_out_dir: str, variants: list) -> None:
    """
    Analyze build directories of selected sample variants and generate report.

    Generate size report in the RST format for each selected board and sample
    variant. The statistics are generated based on partitions.yml file and
    section sizes of zephyr.elf file.
    """
    report = ReportGenerator()

    for board in BOARDS:
        report.add_board(board)

        for variant in variants:
            build_dir = Path(twister_out_dir) / board / \
                variant.full_test_name()

            if not (build_dir / 'zephyr' / 'zephyr.elf').exists():
                continue

            elf = ElfSizeParser(build_dir / 'zephyr' / 'zephyr.elf')
            partitions = PartitionParser(build_dir / 'partitions.yml')
            total_rom = elf.region_size_kb('text', 'data') + partitions.region_size_kb(
                'mcuboot',
                'settings_storage',
                'sidewalk_storage',
                'mfg_storage'
            )

            report.add_variant(SampleVariantStats(sample=variant.full_title(),
                               mcuboot_rom=partitions.region_size_kb(
                                   'mcuboot', 'settings_storage'),
                               app_rom=elf.region_size_kb('text', 'data'),
                               settings=partitions.region_size_kb(
                                   'sidewalk_storage', 'mfg_storage'),
                               total_rom=total_rom,
                               total_ram=elf.region_size_kb('bss', 'data')))

        report.flush_variants()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--twister-out-dir', required=True,
                        help='Path to twister output directory')
    parser.add_argument('--ncs-dir', required=True,
                        help='Path to nRF Connect SDK directory')

    args = parser.parse_args()
    twister_path = str(Path(args.ncs_dir) / 'zephyr' / 'scripts' / 'twister')
    variants = list(select_sample_variants(
        twister_path, args.twister_out_dir, args.ncs_dir))

    build_report(args.twister_out_dir, variants)

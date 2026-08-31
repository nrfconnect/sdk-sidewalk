#!/usr/bin/env python3
#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

"""Generate ``doc/includes/memory_requirement.txt`` from Twister builds.

The script builds the Hello Sidewalk variants of the Sidewalk End Device sample
with a single Twister invocation, measures the footprint of every built image
and renders the reStructuredText tables included by :ref:`sidewalk_requirements`.

Columns of the generated table:

* MCUboot ROM         - ROM used by the ``mcuboot`` sysbuild image.
* Application ROM     - ROM used by the default (application) sysbuild image.
* Factory data        - size of the ``mfg_storage`` partition.
* Settings (Sidewalk) - size of the ``storage`` partition.
* Total ROM           - sum of the four columns above.
* Total RAM           - RAM used by the default (application) sysbuild image.

ROM and RAM are measured with the Twister size calculator, so the values match
the memory usage reported by CI for the same commit. Twister itself reports the
application image only, hence the bootloader is measured separately.

Tabs that are not listed in ``TARGETS`` are kept untouched in the include file,
so a single board can be refreshed without rebuilding all of them. Use
``--rewrite`` to drop them instead.

Run it from the west workspace directory, in the toolchain matching west.yml:

    nrfutil sdk-manager toolchain launch --shell \
        --toolchain-bundle-id "$(nrf/scripts/print_toolchain_checksum.sh)"
    ./sidewalk/scripts/ci/generate_memory_requirement.py

Add ``--background`` to leave most of the CPU to other tasks.
"""

import argparse
import logging
import os
import re
import shlex
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path

import yaml

SIDEWALK_DIR = Path(__file__).resolve().parents[2]
WORKSPACE_DIR = SIDEWALK_DIR.parent
ZEPHYR_BASE = Path(os.environ.setdefault(
    "ZEPHYR_BASE", str(WORKSPACE_DIR / "zephyr")))

# The Twister size calculator is the reference implementation of the footprint
# reported by CI, reuse it instead of reimplementing the section accounting.
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "pylib" / "twister"))
from twisterlib.size_calc import SizeCalculator  # noqa: E402

# ---------------------------------------------------------------------------
# Report configuration
# ---------------------------------------------------------------------------

# Labels of the rows, shared by all boards. The variant is selected by the
# Twister test case, see samples/sid_end_device/sample.yaml.
HELLO_REF = ":ref:`Hello Sidewalk Bluetooth LE <variant_sidewalk_hello>`"
HELLO_SUBGHZ_REF = ":ref:`Hello Sidewalk Bluetooth LE and sub-GHz <variant_sidewalk_hello>`"
BLE_DEBUG = f"{HELLO_REF} (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Debug)"
BLE_RELEASE = f"{HELLO_REF} (``CONFIG_SIDEWALK_SUBGHZ_SUPPORT=n``, Release)"
SUBGHZ_DEBUG = f"{HELLO_SUBGHZ_REF} (Debug)"
SUBGHZ_RELEASE = f"{HELLO_SUBGHZ_REF} (Release)"

# Sub-GHz shields, pick the one matching the test case of the row.
SHIELD_SX1262 = "simple_arduino_adapter;semtech_sx1262mb2cas"
SHIELD_LR1110 = "simple_arduino_adapter;semtech_lr1110mb1xxs"
SHIELD_NRF_SIDEWALK_EB = "nrf_sidewalk_eb"


@dataclass(frozen=True)
class Row:
    """Single row of a memory requirement table."""

    label: str
    """Content of the ``Sample`` column, in reStructuredText."""

    test: str
    """Twister test case name, as defined in sample.yaml."""

    shield: str = ""
    """Shield used by the test case. Informative, set by sample.yaml."""


@dataclass(frozen=True)
class Target:
    """Single tab of the memory requirement include file."""

    board: str
    """Twister platform (board target) to build."""

    tab: str
    """Tab title."""

    description: str
    """End of the first sentence, describing where the sample runs."""

    rows: tuple[Row, ...]
    """Rows of the table, in the order they are rendered."""


TARGETS: tuple[Target, ...] = (
    Target(
        board="nrf54lm20dk/nrf54lm20a/cpuapp",
        tab="nRF54LM20 DK",
        description="running on the `nRF54LM20 DK`_",
        rows=(
            Row(BLE_DEBUG, "sample.sidewalk.hello.ble_only"),
            Row(BLE_RELEASE, "sample.sidewalk.hello.ble_only.release"),
            Row(SUBGHZ_DEBUG, "sample.sidewalk.hello.nrf_sidewalk_eb",
                SHIELD_NRF_SIDEWALK_EB),
            Row(SUBGHZ_RELEASE, "sample.sidewalk.hello.release.nrf_sidewalk_eb",
                SHIELD_NRF_SIDEWALK_EB),
        ),
    ),
    Target(
        board="nrf54l15dk/nrf54l15/cpuapp",
        tab="nRF54L15 DK",
        description="running on the `nRF54L15 DK`_",
        rows=(
            Row(BLE_DEBUG, "sample.sidewalk.hello.ble_only"),
            Row(BLE_RELEASE, "sample.sidewalk.hello.ble_only.release"),
            Row(SUBGHZ_DEBUG, "sample.sidewalk.hello.lr1110", SHIELD_LR1110),
            Row(SUBGHZ_RELEASE, "sample.sidewalk.hello.release.lr1110", SHIELD_LR1110),
        ),
    ),
    Target(
        board="nrf54l15dk/nrf54l10/cpuapp",
        tab="nRF54L10 emulation on nRF54L15 DK",
        description="emulating the nRF54L10 SoC on the `nRF54L15 DK`_",
        rows=(
            Row(BLE_DEBUG, "sample.sidewalk.hello.ble_only"),
            Row(BLE_RELEASE, "sample.sidewalk.hello.ble_only.release"),
        ),
    ),
    Target(
        board="nrf54lv10dk/nrf54lv10a/cpuapp",
        tab="nRF54LV10 DK",
        description="running on the `nRF54LV10 DK`_",
        rows=(
            Row(BLE_DEBUG, "sample.sidewalk.hello.ble_only"),
            Row(BLE_RELEASE, "sample.sidewalk.hello.ble_only.release"),
        ),
    ),
    Target(
        board="nrf52840dk/nrf52840",
        tab="nRF52840 DK",
        description="running on the `nRF52840 DK`_",
        rows=(
            Row(BLE_DEBUG, "sample.sidewalk.hello.ble_only"),
            Row(BLE_RELEASE, "sample.sidewalk.hello.ble_only.release"),
            Row(SUBGHZ_DEBUG, "sample.sidewalk.hello.lr1110", SHIELD_LR1110),
            Row(SUBGHZ_RELEASE, "sample.sidewalk.hello.release.lr1110", SHIELD_LR1110),
        ),
    ),
)

# Partitions reported in the table, by devicetree label.
FACTORY_DATA_PARTITION = "mfg_storage"
SETTINGS_PARTITION = "storage_partition"

# Sysbuild image holding the bootloader.
MCUBOOT_IMAGE = "mcuboot"

# Board revision of a canonical platform name, '@0.7.0' in
# 'nrf54lv10dk@0.7.0/nrf54lv10a/cpuapp'.
REVISION_RE = re.compile(r"@[^_]+")

DOC_INCLUDE = Path("doc/includes/memory_requirement.txt")
TESTSUITE_ROOT = Path("sidewalk")
DEFAULT_OUTDIR = Path("twister-out-memory-requirement")

# Resources left to Twister by --background: a fraction of the CPU threads and
# a lower scheduling priority, so the machine stays usable for other work.
BACKGROUND_CPU_FRACTION = 4
BACKGROUND_NICENESS = 10

# Geometry of the generated table, kept in sync with the reviewed include file.
HEADERS = (
    "Sample",
    "MCUboot ROM [kB]",
    "Application ROM [kB]",
    "Factory data [kB]",
    "Settings (Sidewalk) [kB]",
    "Total ROM [kB]",
    "Total RAM [kB]",
)
COLUMN_WIDTHS = (121, 18, 22, 19, 26, 16, 16)
MIN_PADDING = 2
TABLE_INDENT = 6
TAB_INDENT = 3

KB = 1024

logger = logging.getLogger(Path(__file__).stem)

# ---------------------------------------------------------------------------
# Measurements
# ---------------------------------------------------------------------------

PARTITION_RE = r"^\s*{label}:\s*partition@[0-9a-fA-F]+\s*\{{(?P<body>.*?)^\s*\}};"
REG_RE = re.compile(
    r"reg\s*=\s*<\s*(?:0x[0-9a-fA-F]+|\d+)\s+(?P<size>0x[0-9a-fA-F]+|\d+)\s*>"
)


def partition_size(dts: Path, label: str) -> int:
    """Get the size of a partition from a generated devicetree.

    @param dts Path to zephyr.dts of a build directory.
    @param label Devicetree label of the partition node.
    @return Size of the partition, in bytes.
    """
    node = re.search(
        PARTITION_RE.format(label=re.escape(label)),
        dts.read_text(errors="replace"),
        re.MULTILINE | re.DOTALL,
    )
    if not node:
        raise RuntimeError(f"Partition '{label}' not found in {dts}")

    reg = REG_RE.search(node.group("body"))
    if not reg:
        raise RuntimeError(
            f"Partition '{label}' in {dts} has no 'reg' property")

    return int(reg.group("size"), 0)


def image_footprint(build_dir: Path) -> tuple[int, int]:
    """Measure the memory used by a single image.

    @param build_dir Build directory of the image (a sysbuild domain).
    @return Used ROM and used RAM, in bytes.
    """
    elf = build_dir / "zephyr" / "zephyr.elf"
    if not elf.exists():
        raise RuntimeError(f"Missing {elf}, the image was not linked")

    size = SizeCalculator(
        elf_filename=str(elf),
        extra_sections=[],
        buildlog_filepath="",
        generate_warning=False,
    )
    return size.get_used_rom(), size.get_used_ram()


@dataclass
class Measurement:
    """Memory requirements of a single sample variant, in bytes."""

    mcuboot_rom: int
    app_rom: int
    factory_data: int
    settings: int
    app_ram: int

    @property
    def total_rom(self) -> int:
        return self.mcuboot_rom + self.app_rom + self.factory_data + self.settings


def board_directories(outdir: Path) -> list[Path]:
    """List the per board output directories of a Twister run.

    @param outdir Twister output directory.
    @return Directories, sorted by name.
    """
    return sorted(path for path in outdir.iterdir() if path.is_dir())


def find_board_dir(outdir: Path, board: str) -> Path:
    """Locate the Twister output directory of a single board.

    Twister names it after the canonical platform name, which holds the board
    revision even when TARGETS does not, for example
    ``nrf54lv10dk@0.7.0_nrf54lv10a_cpuapp``.

    @param outdir Twister output directory.
    @param board Twister platform.
    @return Path to the output directory of the board.
    """
    sanitized = board.replace("/", "_")
    if (exact := outdir / sanitized).is_dir():
        return exact

    revisionless = REVISION_RE.sub("", sanitized)
    candidates = [path for path in board_directories(outdir)
                  if REVISION_RE.sub("", path.name) == revisionless]
    if not candidates:
        raise RuntimeError(
            f"No build of '{board}' found in {outdir}, built boards: "
            f"{[path.name for path in board_directories(outdir)]}"
        )
    if len(candidates) > 1:
        raise RuntimeError(
            f"Several revisions of '{board}' were built: "
            f"{[path.name for path in candidates]}. Add the revision to TARGETS."
        )

    return candidates[0]


def find_build_dir(outdir: Path, board: str, test: str) -> Path:
    """Locate the Twister build directory of a single test case.

    @param outdir Twister output directory.
    @param board Twister platform the test case was built for.
    @param test Twister test case name.
    @return Path to the top level (sysbuild) build directory.
    """
    board_dir = find_board_dir(outdir, board)
    candidates = [path.parent for path in board_dir.rglob(
        f"{test}/domains.yaml")]
    if not candidates:
        raise RuntimeError(
            f"No build of '{test}' for '{board}' found in {board_dir}. Check "
            f"{outdir / 'twister.log'} - sample.yaml may filter this test case "
            "out for this board."
        )
    if len(candidates) > 1:
        raise RuntimeError(
            f"Ambiguous builds of '{test}' for '{board}': {candidates}")

    return candidates[0]


def measure(build_dir: Path) -> Measurement:
    """Collect the memory requirements of a single Twister build.

    @param build_dir Top level (sysbuild) build directory.
    @return Memory requirements of the built variant.
    """
    domains = yaml.safe_load((build_dir / "domains.yaml").read_text())
    app_image = domains["default"]

    mcuboot_rom, _ = image_footprint(build_dir / MCUBOOT_IMAGE)
    app_rom, app_ram = image_footprint(build_dir / app_image)
    dts = build_dir / app_image / "zephyr" / "zephyr.dts"

    return Measurement(
        mcuboot_rom=mcuboot_rom,
        app_rom=app_rom,
        factory_data=partition_size(dts, FACTORY_DATA_PARTITION),
        settings=partition_size(dts, SETTINGS_PARTITION),
        app_ram=app_ram,
    )


# ---------------------------------------------------------------------------
# Build
# ---------------------------------------------------------------------------


def background_jobs() -> int:
    """Get the number of Twister jobs that leave the machine responsive.

    @return Number of parallel jobs, at least one.
    """
    return max(1, (os.cpu_count() or 1) // BACKGROUND_CPU_FRACTION)


def lower_priority() -> None:
    """Lower the scheduling priority. Runs in the Twister process."""
    os.nice(BACKGROUND_NICENESS)


def build(outdir: Path, extra_args: list[str], background: bool = False) -> None:
    """Build every test case of every target with a single Twister run.

    Twister has no per-board test case filter, so the boards and the test cases
    are passed as two independent lists. Combinations not supported by
    sample.yaml are filtered out by Twister.

    @param outdir Twister output directory.
    @param extra_args Additional arguments forwarded to Twister.
    @param background Limit the resources used by the build.
    """
    boards = sorted({target.board for target in TARGETS})
    tests = sorted({row.test for target in TARGETS for row in target.rows})

    command = [
        "west",
        "twister",
        "--testsuite-root",
        str(TESTSUITE_ROOT),
        "--outdir",
        str(outdir),
        "--clobber-output",
        "--build-only",
        "--overflow-as-errors",
        "--enable-size-report",
        "--show-footprint",
        "--inline-logs",
    ]
    for board in boards:
        command += ["--platform", board]
    for test in tests:
        command += ["--test", test]
    if background:
        command += ["--jobs", str(background_jobs())]
    # Forwarded last, so that they override the options set above.
    command += extra_args

    # os.nice() is not available on Windows, where the build runs at the
    # default priority and only the job count is limited.
    preexec_fn = lower_priority if background and hasattr(os, "nice") else None

    logger.info("Building %d test case(s) for %d board(s)",
                len(tests), len(boards))
    if background:
        logger.info("Limited to %d job(s)%s", background_jobs(),
                    f", niceness +{BACKGROUND_NICENESS}" if preexec_fn else "")
    logger.info("%s", shlex.join(command))
    subprocess.run(command, cwd=WORKSPACE_DIR, check=True,
                   preexec_fn=preexec_fn)


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------


def format_size(value: int) -> str:
    """Format a measured byte count as kilobytes.

    @param value Value to format, in bytes.
    @return Value in kB, with two decimal places.
    """
    return f"{value / KB:.2f}"


def format_partition(value: int) -> str:
    """Format a partition size as kilobytes.

    @param value Value to format, in bytes.
    @return Value in kB, without a redundant fraction.
    """
    kilobytes = value / KB
    return f"{kilobytes:.0f}" if kilobytes.is_integer() else f"{kilobytes:.2f}"


def render_separator(widths: tuple[int, ...], char: str = "-") -> str:
    """Render a table separator line.

    @param widths Width of every column.
    @param char Fill character, '=' below the header.
    @return The separator, including the leading indentation.
    """
    return " " * TABLE_INDENT + "+" + "+".join(char * width for width in widths) + "+"


def render_header(widths: tuple[int, ...]) -> str:
    """Render the header row of the table.

    @param widths Width of every column.
    @return The row, including the leading indentation.
    """
    cells = [f" {HEADERS[0]}".ljust(widths[0])]
    cells += [f" {name} ".center(width)
              for name, width in zip(HEADERS[1:], widths[1:])]
    return " " * TABLE_INDENT + "|" + "|".join(cells) + "|"


def render_row(label: str, values: tuple[str, ...], widths: tuple[int, ...]) -> str:
    """Render a single data row of the table.

    @param label Content of the ``Sample`` column.
    @param values Content of the remaining columns.
    @param widths Width of every column.
    @return The row, including the leading indentation.
    """
    cells = [label.center(widths[0])]
    cells += [value.center(width) for value, width in zip(values, widths[1:])]
    return " " * TABLE_INDENT + "|" + "|".join(cells) + "|"


def column_widths(target: Target) -> tuple[int, ...]:
    """Get the column widths, widened if the labels do not fit.

    @param target Target the table is rendered for.
    @return Width of every column.
    """
    labels = max(len(row.label) for row in target.rows)
    return (max(COLUMN_WIDTHS[0], labels + 2 * MIN_PADDING), *COLUMN_WIDTHS[1:])


def render_table(target: Target, measurements: dict[str, Measurement]) -> list[str]:
    """Render the memory requirement table of a single target.

    @param target Target to render.
    @param measurements Measurement of every test case of the target.
    @return Lines of the table.
    """
    widths = column_widths(target)
    lines = [
        render_separator(widths),
        render_header(widths),
        render_separator(widths, "="),
    ]
    for row in target.rows:
        result = measurements[row.test]
        values = (
            format_size(result.mcuboot_rom),
            format_size(result.app_rom),
            format_partition(result.factory_data),
            format_partition(result.settings),
            format_size(result.total_rom),
            format_size(result.app_ram),
        )
        lines += [render_row(row.label, values, widths),
                  render_separator(widths)]

    return lines


def render_tab(target: Target, measurements: dict[str, Measurement]) -> str:
    """Render the tab of a single target.

    @param target Target to render.
    @param measurements Measurement of every test case of the target.
    @return The tab directive with its table.
    """
    body = " " * TABLE_INDENT
    lines = [
        f"{' ' * TAB_INDENT}.. tab:: {target.tab}",
        "",
        f"{body}The following table lists the total memory requirements for the default "
        f"variant of the :ref:`Sidewalk_End_device` sample {target.description}.",
        f"{body}To determine the available space for user extension, it needs to be "
        "summarized against the allocated partition size and its layout.",
        "",
        *render_table(target, measurements),
    ]
    return "\n".join(lines)


TAB_RE = re.compile(r"^\s*\.\. tab:: (?P<title>.+)$")


def split_tabs(text: str) -> dict[str, str]:
    """Split an include file into its tabs.

    @param text Content of the include file.
    @return Tab body by tab title, in the order of appearance.
    """
    tabs: dict[str, str] = {}
    title = None
    body: list[str] = []

    for line in text.splitlines():
        match = TAB_RE.match(line)
        if match:
            if title:
                tabs[title] = "\n".join(body).rstrip()
            title = match.group("title").strip()
            body = [line]
        elif title:
            body.append(line)

    if title:
        tabs[title] = "\n".join(body).rstrip()

    return tabs


def render_include(tabs: dict[str, str]) -> str:
    """Render the complete include file.

    @param tabs Tab body by tab title, in the order they are rendered.
    @return Content of the include file.
    """
    return ".. tabs::\n\n" + "\n\n".join(tabs.values()) + "\n"


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def get_arguments() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate the memory requirement include file of the Sidewalk "
        "documentation.",
        epilog="Arguments after a '--' separator are forwarded to Twister.",
    )
    parser.add_argument(
        "-o", "--outdir", type=Path, default=DEFAULT_OUTDIR,
        help=f"Twister output directory (default: {DEFAULT_OUTDIR})",
    )
    parser.add_argument(
        "--no-build", action="store_true",
        help="reuse the builds already present in the output directory",
    )
    parser.add_argument(
        "--background", action="store_true",
        help="build with limited resources, so that the machine stays usable "
             "for other tasks",
    )
    parser.add_argument(
        "--rewrite", action="store_true",
        help="drop the tabs of the boards that are not listed in TARGETS",
    )
    parser.add_argument(
        "--dry-run", action="store_true",
        help="print the generated file instead of updating it",
    )
    return parser


def split_argv(argv: list[str]) -> tuple[list[str], list[str]]:
    """Split the command line on the ``--`` separator.

    Forwarding unknown arguments instead would silently accept a misspelled
    option of this script, and only report it after the build.

    @param argv Command line, without the program name.
    @return Arguments of this script and arguments forwarded to Twister.
    """
    if "--" not in argv:
        return argv, []

    separator = argv.index("--")
    return argv[:separator], argv[separator + 1:]


def main() -> int:
    logging.basicConfig(
        format="%(levelname)-8s %(message)s", level=logging.INFO)

    argv, extra_args = split_argv(sys.argv[1:])
    options = get_arguments().parse_args(argv)

    outdir = options.outdir
    if not outdir.is_absolute():
        outdir = WORKSPACE_DIR / outdir

    if options.no_build and extra_args:
        logger.error("Nothing is built, cannot forward: %s",
                     shlex.join(extra_args))
        return 1
    if not options.no_build:
        build(outdir, extra_args, options.background)

    include = SIDEWALK_DIR / DOC_INCLUDE
    tabs = {} if options.rewrite else split_tabs(include.read_text())

    for target in TARGETS:
        measurements = {}
        for row in target.rows:
            result = measure(find_build_dir(outdir, target.board, row.test))
            measurements[row.test] = result
            logger.info(
                "%s: %s%s -> ROM %s kB, RAM %s kB",
                target.board,
                row.test,
                f" (shield {row.shield})" if row.shield else "",
                format_size(result.total_rom),
                format_size(result.app_ram),
            )
        tabs[target.tab] = render_tab(target, measurements)

    content = render_include(tabs)

    if options.dry_run:
        print(content, end="")
        return 0

    include.write_text(content)
    logger.info("Updated %s", DOC_INCLUDE)
    return 0


if __name__ == "__main__":
    sys.exit(main())

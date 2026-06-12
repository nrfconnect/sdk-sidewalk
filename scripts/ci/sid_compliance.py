# Copyright (c) 2022 Nordic Semiconductor ASA
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import re
import sys
import os
import subprocess
from pathlib import Path


def _resolve_zephyr_base() -> Path:
    """Return Zephyr tree path; used by git hooks where ZEPHYR_BASE is often unset."""

    def is_zephyr_root(p: Path) -> bool:
        return (p / "scripts" / "ci" / "check_compliance.py").is_file()

    env = os.environ.get("ZEPHYR_BASE")
    if env:
        candidate = Path(env).expanduser().resolve()
        if is_zephyr_root(candidate):
            return candidate
        print(
            f"sid_compliance: ZEPHYR_BASE={env!r} does not look like a Zephyr tree "
            "(missing scripts/ci/check_compliance.py).",
            file=sys.stderr,
        )
        sys.exit(1)

    # West workspace default: zephyr/ next to sidewalk/ (this repo).
    sidewalk_root = Path(__file__).resolve().parents[2]
    sibling = (sidewalk_root.parent / "zephyr").resolve()
    if is_zephyr_root(sibling):
        os.environ["ZEPHYR_BASE"] = str(sibling)
        return sibling

    print(
        "sid_compliance: ZEPHYR_BASE is not set and ../zephyr was not found next to "
        "this repository. Export ZEPHYR_BASE to your Zephyr tree, or use an NCS shell "
        "that sets it.",
        file=sys.stderr,
    )
    sys.exit(1)


ZEPHYR_BASE = _resolve_zephyr_base()
sys.path.insert(0, str(ZEPHYR_BASE / "scripts" / "ci"))

# autopep8: off
import check_compliance as cc  # noqa: E402
# autopep8: on


def _git_supports_perl_regexes() -> bool:
    """
    GitHub Actions runners can use a git build without PCRE support.
    Zephyr check_compliance uses `git grep --perl-regexp` with lookaheads in
    Kconfig checks; if unsupported, git exits 128 with a fatal message.
    """
    repo_root = Path(__file__).resolve().parents[2]
    proc = subprocess.run(
        [
            "git",
            "grep",
            "--line-number",
            "-I",
            "--null",
            "--perl-regexp",
            r"\bCONFIG_[A-Z0-9_]+\b",
            "--",
            ":boards",
        ],
        cwd=repo_root,
        capture_output=True,
        text=True,
        check=False,
    )
    if proc.returncode == 128 and "USE_LIBPCRE" in (proc.stderr or ""):
        return False
    return True


if not _git_supports_perl_regexes():
    # In this environment, `git grep --perl-regexp` can't run, which makes
    # Zephyr's `check_disallowed_defconfigs()` crash the entire KconfigBasic
    # check. Skip only that sub-check so the rest of KconfigBasic is still
    # enforced.
    cc.KconfigCheck.check_disallowed_defconfigs = lambda self, kconf: None  # noqa: E731

SIDEWALK_ROOT = Path(__file__).resolve().parents[2]
_REFERENCED_AT_RE = re.compile(r"Referenced at (.+?):\d+:")


def _referenced_paths(warning_block: str) -> list[Path]:
    paths = []
    for match in _REFERENCED_AT_RE.finditer(warning_block):
        try:
            paths.append(Path(match.group(1)).resolve())
        except OSError:
            paths.append(Path(match.group(1)))
    return paths


def _sidewalk_kconfig_undef_warnings(warnings: str) -> str:
    """
    Keep undefined-symbol warnings only when Sidewalk Kconfig references them.

    The trimmed west manifest does not import every NCS module. KconfigBasic
    still parses the full NCS tree, so missing optional modules can surface as
    undefined symbols in nrf/ or zephyr/ Kconfig. Those are not Sidewalk bugs.
    """
    if not warnings.strip():
        return ""

    blocks = re.split(r"(?=\n warning: undefined symbol)", "\n" + warnings)
    kept = []

    for block in blocks:
        block = block.strip()
        if not block:
            continue

        refs = _referenced_paths(block)
        if not refs:
            kept.append(block)
            continue

        if any(
            str(path).startswith(str(SIDEWALK_ROOT) + os.sep) for path in refs
        ):
            kept.append(block)

    return "\n\n\n".join(kept)


def _scoped_check_no_undef_within_kconfig(self, kconf):
    undef_ref_warnings = "\n\n\n".join(
        warning for warning in kconf.warnings if "undefined symbol" in warning
    )
    undef_ref_warnings = _sidewalk_kconfig_undef_warnings(undef_ref_warnings)

    if undef_ref_warnings:
        self.failure(f"Undefined Kconfig symbols:\n\n {undef_ref_warnings}")


cc.KconfigBasicCheck.check_no_undef_within_kconfig = _scoped_check_no_undef_within_kconfig
cc.SysbuildKconfigBasicCheck.check_no_undef_within_kconfig = (
    _scoped_check_no_undef_within_kconfig
)

sidewalk_specials = [
    r"tools/.*",
    r"subsys/ace(/.*)+h",
    r"subsys/config(/.*)+(c|h)",
    r"subsys/config/common/.*(c|h)",
    r"subsys/config/common/src/.*(c|h)",
    r"subsys/hal(/.*)+h",
    r"subsys/hal/src/memory.c",
    r"subsys/sal/common(/.*)+h",
    r"tests/validation/storage_kv/.*(c|h)",
    r"tests/validation/timer/.*(c|h)",
    r"subsys/demo(/.*)+(c|h)",
    r"subsys/semtech/.*",
]


def check_for_special_files(file):
    return not any(re.search(regex, file) for regex in sidewalk_specials)


def python_filter(files):
    return list(filter(check_for_special_files, files))


def get_files_overwrite(filter=None, paths=None):
    filter_arg = (f"--diff-filter={filter}",) if filter else ()
    paths_arg = ("--", *paths) if paths else ()
    out = cc.git("diff", "--name-only", *filter_arg, cc.COMMIT_RANGE, *paths_arg)
    files = out.splitlines()
    for file in list(files):
        if not os.path.isfile(os.path.join(cc.GIT_TOP, file)):
            # Drop submodule directories from the list.
            files.remove(file)

    return python_filter(files)


cc.get_files = get_files_overwrite


def binaryfiles_run_overwrite(self):
    BINARY_ALLOW_PATHS = ("doc/", "lib/")
    # svg files are always detected as binary, see .gitattributes
    BINARY_ALLOW_EXT = (".jpg", ".jpeg", ".png", ".svg", ".webp", ".a")

    for stat in cc.git(
        "diff", "--numstat", "--diff-filter=A", cc.COMMIT_RANGE
    ).splitlines():
        added, deleted, fname = stat.split("\t")
        if added == "-" and deleted == "-":
            if fname.startswith(BINARY_ALLOW_PATHS) and fname.endswith(
                BINARY_ALLOW_EXT
            ):
                continue
            self.failure(f"Binary file not allowed: {fname}")


cc.BinaryFiles.run = binaryfiles_run_overwrite

if __name__ == "__main__":
    argv = sys.argv[1:]

    # These checks validate Zephyr Kconfig parsing with *modules removed*.
    # In an NCS west workspace (like this one), they flag NCS-specific symbols
    # as undefined and are not actionable for Sidewalk itself.
    if "-e" not in argv and "--exclude-module" not in argv:
        argv = [
            *argv,
            "--exclude-module",
            "KconfigBasicNoModules",
            "--exclude-module",
            "SysbuildKconfigBasicNoModules",
            # Full Kconfig scans reference NCS/Zephyr symbols outside Sidewalk.
            "--exclude-module",
            "Kconfig",
            "--exclude-module",
            "SysbuildKconfig",
        ]

    cc.main(argv)

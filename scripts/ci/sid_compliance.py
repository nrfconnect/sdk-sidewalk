# Copyright (c) 2022 Nordic Semiconductor ASA
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

import re
import sys
import os
from pathlib import Path

ZEPHYR_BASE = os.environ.get('ZEPHYR_BASE')
if ZEPHYR_BASE is None:
    # Infer from west workspace: sidewalk/scripts/ci/sid_compliance.py → ../../../zephyr
    ZEPHYR_BASE = str(Path(__file__).resolve().parents[2] / 'zephyr')
    os.environ['ZEPHYR_BASE'] = ZEPHYR_BASE

# Git sets GIT_DIR/GIT_INDEX_FILE when running hooks. These variables
# propagate to subprocess git calls and make git grep search the wrong
# repo (sidewalk instead of zephyr) when cwd=ZEPHYR_BASE is used. Unset
# them so each subprocess auto-detects its repo from its working directory.
for _git_env_var in ('GIT_DIR', 'GIT_INDEX_FILE', 'GIT_WORK_TREE'):
    os.environ.pop(_git_env_var, None)

sys.path.insert(0, str(Path(ZEPHYR_BASE, 'scripts', 'ci').resolve()))

# autopep8: off
import check_compliance as cc
# autopep8: on

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
    filter_arg = (f'--diff-filter={filter}',) if filter else ()
    paths_arg = ('--', *paths) if paths else ()
    out = cc.git('diff', '--name-only', *filter_arg,
                 cc.COMMIT_RANGE, *paths_arg)
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

    for stat in cc.git("diff", "--numstat", "--diff-filter=A",
                       cc.COMMIT_RANGE).splitlines():
        added, deleted, fname = stat.split("\t")
        if added == "-" and deleted == "-":
            if (fname.startswith(BINARY_ALLOW_PATHS) and
                    fname.endswith(BINARY_ALLOW_EXT)):
                continue
            self.failure(f"Binary file not allowed: {fname}")


cc.BinaryFiles.run = binaryfiles_run_overwrite


if hasattr(cc.KconfigBasicCheck, 'check_disallowed_defconfigs'):
    _orig_check_disallowed_defconfigs = cc.KconfigBasicCheck.check_disallowed_defconfigs

    def check_disallowed_defconfigs_overwrite(self, kconf):
        # check_disallowed_defconfigs uses git grep --perl-regexp, which requires
        # git compiled with USE_LIBPCRE. Skip gracefully when PCRE is unavailable
        # (e.g. when the NCS toolchain git is first in PATH).
        try:
            _orig_check_disallowed_defconfigs(self, kconf)
        except SystemExit as e:
            if "USE_LIBPCRE" in str(e):
                return
            raise

    cc.KconfigBasicCheck.check_disallowed_defconfigs = check_disallowed_defconfigs_overwrite

if __name__ == "__main__":
    cc.main(sys.argv[1:])

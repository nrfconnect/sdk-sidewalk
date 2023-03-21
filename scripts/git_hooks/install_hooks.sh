#!/bin/sh

# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

link_hook()
{
    hook_name=$1
    rm -rf $ZEPHYR_BASE/../sidewalk/.git/hooks/$hook_name

    ln -s $ZEPHYR_BASE/../sidewalk/scripts/git_hooks/$hook_name  $ZEPHYR_BASE/../sidewalk/.git/hooks/$hook_name
    chmod +x $ZEPHYR_BASE/../sidewalk/scripts/git_hooks/$hook_name
}

link_hook commit-msg
link_hook pre-commit
link_hook prepare-commit-msg

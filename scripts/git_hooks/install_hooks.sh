#!/bin/sh

link_hook()
{
    hook_name=$1
    if test -f "$ZEPHYR_BASE/../sidewalk/.git/hooks/$hook_name"; then
        rm -rf $ZEPHYR_BASE/../sidewalk/.git/hooks/$hook_name
    fi

    ln -s $ZEPHYR_BASE/../sidewalk/scripts/git_hooks/$hook_name  $ZEPHYR_BASE/../sidewalk/.git/hooks/$hook_name
    chmod +x $ZEPHYR_BASE/../sidewalk/scripts/git_hooks/$hook_name
}

link_hook commit-msg
link_hook pre-commit
link_hook prepare-commit-msg

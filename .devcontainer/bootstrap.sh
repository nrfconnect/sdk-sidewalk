#!/bin/bash -x
if [ ! -f $HOME/.devcontainer_initalized ]; then
    bash /work/ncs/sidewalk/.devcontainer/git_shell_prompt.sh

    cd /work/ncs
    west init -l --mf internal_west.yml sidewalk 
    git submodule --init --recursive
    west config update.path-cache /workdir
    west config update.narrow true
    west update
    bash /work/ncs/sidewalk/scripts/git_hooks/install_hooks.sh
    pip install -r /work/ncs/sidewalk/requirements.txt

    touch $HOME/.devcontainer_initalized
fi

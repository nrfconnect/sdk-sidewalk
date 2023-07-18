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

    if [ ! -f /work/ncs/sidewalk/doc/venv/bin/activate ]; then
        cd /work/ncs/sidewalk/doc/
        python3 -m venv venv
        /bin/bash -c ". /work/ncs/sidewalk/doc/venv/bin/activate && pip install -r /work/ncs/sidewalk/doc/requirements-doc.txt"
    fi
    touch $HOME/.devcontainer_initalized
fi

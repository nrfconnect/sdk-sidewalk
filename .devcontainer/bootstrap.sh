#!/bin/bash -x
if [ ! -f /devcontainer_initalized ]; then
    bash ~/work/ncs/sidewalk/.devcontainer/git_shell_prompt.sh

    cd ~/work/ncs
    west init -l sidewalk
    west update -n -o=--depth=1 --path-cache=/cache
    bash ~/work/ncs/sidewalk/scripts/git_hooks/install_hooks.sh
    pip install -r ~/work/ncs/sidewalk/requirements.txt

    cd ~/work/ncs/sidewalk/doc/
    python3 -m venv venv
    source venv/bin/activate
    pip install -r ~/work/ncs/sidewalk/doc/requirements-doc.txt
    deactivate

    touch /devcontainer_initalized
fi

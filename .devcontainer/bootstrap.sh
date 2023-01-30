#!/bin/bash -x
bash ~/work/ncs/sidewalk/.devcontainer/git_shell_prompt.sh

cd ~/work/ncs/sidewalk
git lfs fetch --all
git lfs pull
cd ..
west init -l sidewalk
west update -n -o=--depth=1
bash ~/work/ncs/sidewalk/scripts/git_hooks/install_hooks.sh
pip install -r ~/work/ncs/sidewalk/requirements.txt
pip install -r ~/work/ncs/sidewalk/doc/requirements-doc.txt

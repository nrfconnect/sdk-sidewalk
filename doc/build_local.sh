#!/usr/bin/env bash
set -e

python3 -m venv venv
source ./venv/bin/activate

pip install -r $(pwd)/requirements-doc.txt

$(pwd)/update_ncs_links.sh > ncs_links.rst

sphinx-build -M html . build

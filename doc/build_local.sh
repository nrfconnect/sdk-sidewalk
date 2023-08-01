#!/usr/bin/env bash
set -e

python3 -m venv venv
source ./venv/bin/activate

pip install -r $(pwd)/requirements-doc.txt

sphinx-build -M html . build

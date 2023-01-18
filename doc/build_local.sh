#!/usr/bin/env bash
set -e
source "./venv/bin/activate"
sphinx-build -M html . build

#!/bin/bash
#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#


# Script to build documentation in python3.10 docker (no need to install anything except docker which is able to run without sudo)
# The scripts pulls official python docker image, starts container and inside container
# installs required Python packages (from requirements-doc.txt) and execute sphinx-build tool.
# HTML documentation is generated to build/html folder

# How to install docker on Ubuntu:
# 1. https://docs.docker.com/engine/install/ubuntu:
# 2. Run docker without sudo:
# 2.1. Issue command: sudo groupadd docker && sudo usermod -aG docker $USER
# 2.2. Log out and log back to activate the group membership in Linux or you can run: newgrp docker

function err_trap () {
    echo "$0: line $1: exit status of last command: $2"
    exit 1
}
set -E
trap 'err_trap ${LINENO} ${$?}' ERR

DOCKER=python:3.10

if [ $# -eq 0 ]; then
    echo "Docker $DOCKER executing"
    docker run -it --rm -e USER_ID=$(id -u) -e GRP_ID=$(id -g) -v "$(pwd)":/work -v "$(pwd)"/build_doc_in_docker.sh:/run.sh "${DOCKER}" /run.sh "RUN_IN_DOCKER"
    echo "Docker exited"
elif [ $# -eq 1 ]; then
    cd /work
    pip install -r requirements-doc.txt
    sphinx-build -M html . build
    chown -R $USER_ID:$GRP_ID build
fi

FROM ubuntu:22.04
LABEL org.opencontainers.image.source="https://github.com/nrfconnect/sdk-sidewalk"
ARG DEBIAN_FRONTEND=noninteractive

ARG tools_version_major=10
ARG tools_version_minor=24
ARG tools_version_patch=2

ARG command_line_tools_name=nrf-command-line-tools_${tools_version_major}.${tools_version_minor}.${tools_version_patch}_amd64.deb
ARG command_line_tools_url=https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${tools_version_major}-x-x/${tools_version_major}-${tools_version_minor}-${tools_version_patch}/${command_line_tools_name}

RUN apt-get update; apt-get install --no-install-recommends -y wget curl libusb-1.0-0 \
  clangd ssh nano bash-completion gpg ruby lcov screen libffi7 libffi-dev libfftw3-dev python3-venv \
  git-lfs golang-go clang-format python3-pip python3-autopep8 gitlint libmagic1 git gcc g++ gcc-multilib lbzip2 jq sudo zip

ENV NRFUTIL_HOME=/usr/local/share/nrfutil
RUN <<EOT
    mkdir -p /usr/local/share/nrfutil
    wget -q https://developer.nordicsemi.com/.pc-tools/nrfutil/x64-linux/nrfutil
    mv nrfutil /usr/local/bin
    chmod +x /usr/local/bin/nrfutil
    nrfutil install toolchain-manager
EOT

RUN wget ${command_line_tools_url}
RUN dpkg -i ${command_line_tools_name}

RUN <<EOT
    echo '#!/bin/bash\necho not running udevadm "$@"' > /usr/bin/udevadm
    chmod +x /usr/bin/udevadm 
    JLink_to_install=`ls /opt/nrf-command-line-tools/share | grep JLink_Linux`
    apt install -y /opt/nrf-command-line-tools/share/$JLink_to_install --fix-broken
EOT

RUN <<EOT
    nrfutil toolchain-manager search > versions.txt
    latest_version=`tail -n+2 versions.txt | sort --reverse | head -n 1 | awk '{print$1}'`
    echo "installing toolchain version" $latest_version
    nrfutil toolchain-manager install --ncs-version $latest_version
    nrfutil toolchain-manager list
EOT

WORKDIR /workdir
SHELL ["nrfutil","toolchain-manager","launch","/bin/bash","--","-c"]
RUN <<EOT
    west init -m https://github.com/nrfconnect/sdk-nrf --mr main .
    west update
    python3 -m pip install devicetree
    python3 -m pip install -r /workdir/nrf/scripts/requirements.txt -r /workdir/zephyr/scripts/requirements.txt 
EOT

# add your commands below

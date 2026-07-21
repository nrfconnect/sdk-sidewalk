FROM ubuntu:24.04
LABEL org.opencontainers.image.source="https://github.com/nrfconnect/sdk-sidewalk"
ARG DEBIAN_FRONTEND=noninteractive

ARG tools_version_major=10
ARG tools_version_minor=24
ARG tools_version_patch=2

ARG NRF_VERSION=main
ARG TOOLCHAIN_BUNDLE_ID
ARG NCS_INSTALL_DIR=/root/ncs

ARG command_line_tools_name=nrf-command-line-tools_${tools_version_major}.${tools_version_minor}.${tools_version_patch}_amd64.deb
ARG command_line_tools_url=https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${tools_version_major}-x-x/${tools_version_major}-${tools_version_minor}-${tools_version_patch}/${command_line_tools_name}

RUN apt-get update; apt-get install --no-install-recommends -y wget curl libusb-1.0-0 \
  clangd ssh nano bash-completion gpg ruby lcov screen libffi-dev libfftw3-dev python3-venv \
  git-lfs golang-go clang-format python3-pip python3-autopep8 gitlint libmagic1 git gcc g++ gcc-multilib lbzip2 jq sudo zip make gdb vim wget unzip \
  && rm -rf /var/lib/apt/lists/*

ENV NRFUTIL_HOME=/usr/local/share/nrfutil
RUN <<EOT
    mkdir -p /usr/local/share/nrfutil
    wget -q https://files.nordicsemi.com/artifactory/swtools/external/nrfutil/executables/x86_64-unknown-linux-gnu/nrfutil -O /usr/local/bin/nrfutil
    chmod +x /usr/local/bin/nrfutil
    nrfutil install sdk-manager
    nrfutil install device
    nrfutil install completion
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
    mkdir /workdir
    git clone https://github.com/nrfconnect/sdk-nrf /workdir/nrf
    cd /workdir/nrf
    git checkout ${NRF_VERSION}
EOT

RUN <<EOT
    BUNDLE_ID="${TOOLCHAIN_BUNDLE_ID}"
    if [ -z "$BUNDLE_ID" ]; then
      BUNDLE_ID=$(/workdir/nrf/scripts/print_toolchain_checksum.sh)
    fi
    nrfutil sdk-manager toolchain install --toolchain-bundle-id "$BUNDLE_ID" --install-dir ${NCS_INSTALL_DIR}
    nrfutil sdk-manager toolchain env --toolchain-bundle-id "$BUNDLE_ID" --install-dir ${NCS_INSTALL_DIR} --as-script sh > /opt/toolchain-env.sh
EOT

ENV BASH_ENV=/opt/toolchain-env.sh
WORKDIR /workdir
SHELL ["/bin/bash", "-c"]
RUN <<EOT
    west init -l nrf
    west update
    python3 -m pip install devicetree
    python3 -m pip install -r /workdir/nrf/scripts/requirements.txt -r /workdir/zephyr/scripts/requirements.txt
EOT

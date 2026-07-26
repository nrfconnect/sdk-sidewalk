FROM ubuntu:24.04
LABEL org.opencontainers.image.source="https://github.com/nrfconnect/sdk-sidewalk"
ARG DEBIAN_FRONTEND=noninteractive

ARG JLINK_VERSION=924a

ARG NRF_VERSION=main
ARG TOOLCHAIN_BUNDLE_ID
ARG NCS_INSTALL_DIR=/root/ncs

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

RUN <<EOT
    echo '#!/bin/bash\necho not running udevadm "$@"' > /usr/bin/udevadm
    chmod +x /usr/bin/udevadm
    JLINK_DEB=JLink_Linux_V${JLINK_VERSION}_x86_64.deb
    wget --post-data="accept_license_agreement=accepted&submit=Download+software" \
        -O ${JLINK_DEB} https://www.segger.com/downloads/jlink/${JLINK_DEB}
    apt install -y ./${JLINK_DEB} --fix-broken
    rm -f ${JLINK_DEB}
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

FROM ubuntu:22.04

ARG UNAME=vscode
ARG UNAME_PASSWORD=pass

ARG ZEPHYR_SDK_VERSION=0.15.2
ARG TOOLS_VERSION=10
ARG TOOLS_VERSION_MAJOR=19
ARG TOOLS_VERSION_MINOR=0
ARG NRF_REVISION=main

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y \
  wget git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file  \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 udev aria2
# Download jlink tools
RUN aria2c -V -d jlink https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_linux-amd64.tar.gz && sync; \
cd jlink; tar -xvf nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_linux-amd64.tar.gz && dpkg -i JLink_Linux_*.deb; apt-get --fix-broken install -y && rm -rf jlink
# Download nrf-command-line-tools
RUN aria2c -V -d command-line https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb && sync; \
cd command-line; dpkg -i nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb; apt-get --fix-broken install -y && rm -rf command-line
# Download zephyr-sdk
RUN aria2c -V -d zephyr-sdk https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZEPHYR_SDK_VERSION}/zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.gz && sync; \
cd zephyr-sdk; tar -xvf zephyr-sdk-${ZEPHYR_SDK_VERSION}_linux-x86_64_minimal.tar.gz && mv zephyr-sdk-${ZEPHYR_SDK_VERSION} /opt && rm -rf zephyr-sdk; \
/opt/zephyr-sdk-${ZEPHYR_SDK_VERSION}/setup.sh -h -c -t x86_64-zephyr-elf -t arm-zephyr-eabi

RUN pip3 install west

RUN mkdir -p /cache; cd /cache; west init -m https://github.com/nrfconnect/sdk-nrf --mr $NRF_REVISION ; west update
RUN pip3 install -r /cache/nrf/scripts/requirements.txt; pip3 install -r /cache/zephyr/scripts/requirements.txt; pip3 install -r /cache/bootloader/mcuboot/scripts/requirements.txt

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y \
  clangd ssh nano bash-completion sudo gpg ruby lcov screen uncrustify libfftw3-dev curl python3-venv git-lfs golang-go

RUN go install github.com/apache/mynewt-mcumgr-cli/mcumgr@latest
RUN cp /root/go/bin/* /usr/local/bin/

RUN useradd -ms /bin/bash $UNAME && \
echo "$UNAME"' ALL=(ALL) NOPASSWD: ALL' > /etc/sudoers.d/$UNAME && \
usermod -aG sudo $UNAME && \
usermod -aG dialout $UNAME && \
echo "$UNAME:$UNAME_PASSWORD" | chpasswd

USER $UNAME


# add your commands below

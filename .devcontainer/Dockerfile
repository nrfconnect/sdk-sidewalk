FROM ubuntu:22.04

ARG UNAME=vscode
ARG UNAME_PASSWORD=pass

ARG SDK_VERSION=0.15.1
ARG TOOLS_VERSION=10
ARG TOOLS_VERSION_MAJOR=19
ARG TOOLS_VERSION_MINOR=0
ARG NRF_REVISION=main

RUN useradd -ms /bin/bash $UNAME

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y \
  wget git cmake ninja-build gperf ccache dfu-util device-tree-compiler \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file  \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 udev aria2
# Download jlink tools
RUN aria2c -V -d jlink https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_linux-amd64.tar.gz && sync
RUN cd jlink; tar -xvf nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_linux-amd64.tar.gz && dpkg -i JLink_Linux_*.deb; apt-get --fix-broken install -y
RUN rm -rf jlink
# Download nrf-command-line-tools
RUN aria2c -V -d command-line https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb && sync
RUN cd command-line; dpkg -i nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb; apt-get --fix-broken install -y
RUN rm -rf command-line
# Download zephyr-sdk
RUN aria2c -V -d zephyr-sdk https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/zephyr-sdk-${SDK_VERSION}_linux-x86_64_minimal.tar.gz && sync
RUN cd zephyr-sdk; tar -xvf zephyr-sdk-${SDK_VERSION}_linux-x86_64_minimal.tar.gz && mv zephyr-sdk-${SDK_VERSION} /opt
RUN rm -rf zephyr-sdk
RUN /opt/zephyr-sdk-${SDK_VERSION}/setup.sh -h -c -t x86_64-zephyr-elf -t arm-zephyr-eabi

# Download Go and mcumgr
RUN aria2c  -V -d go_download https://go.dev/dl/go1.19.3.linux-amd64.tar.gz
RUN cd go_download; tar -C /usr/local -xzf go1.19.3.linux-amd64.tar.gz
RUN rm -rf go_download
RUN echo 'export GOPATH=/home/'"$UNAME" >> /home/$UNAME/.profile
RUN echo 'export PATH=$PATH:/usr/local/go/bin:$GOPATH/bin' >> /home/$UNAME/.profile
RUN . /home/$UNAME/.profile; go install github.com/apache/mynewt-mcumgr-cli/mcumgr@latest

RUN echo 'export PATH=$PATH:/home/'"$UNAME"'/.local/bin' >> /home/$UNAME/.bashrc
RUN pip3 install west

RUN mkdir -p /cache; cd /cache; west init -m https://github.com/nrfconnect/sdk-nrf --mr $NRF_REVISION ; west update -n -o=--depth=1
RUN pip3 install -r /cache/nrf/scripts/requirements.txt; pip3 install -r /cache/zephyr/scripts/requirements.txt; pip3 install -r /cache/bootloader/mcuboot/scripts/requirements.txt

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y \
  clangd ssh nano bash-completion sudo gpg ruby lcov screen uncrustify libfftw3-dev curl python3-venv git-lfs

RUN echo "$UNAME"' ALL=(ALL) NOPASSWD: ALL' > /etc/sudoers.d/$UNAME
RUN usermod -aG sudo $UNAME
RUN usermod -aG dialout $UNAME
RUN echo "$UNAME:$UNAME_PASSWORD" | chpasswd 
USER $UNAME
RUN mkdir -p /home/$UNAME/work/ncs

# add your commands below

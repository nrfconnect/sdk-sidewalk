FROM ubuntu:22.04

ARG UNAME=vscode
ARG UNAME_PASSWORD=pass
ARG SDK_VERSION=0.15.1
ARG TOOLS_VERSION=10
ARG TOOLS_VERSION_MAJOR=17
ARG TOOLS_VERSION_MINOR=3

RUN useradd -ms /bin/bash $UNAME 
RUN usermod -aG sudo $UNAME
RUN usermod -aG dialout $UNAME

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y wget python3-pip git cmake ninja-build gperf  \
  ccache dfu-util device-tree-compiler \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file  \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 udev clangd ssh nano bash-completion sudo gpg ruby lcov screen uncrustify libfftw3-dev

RUN apt-get install --no-install-recommends -y aria2
RUN aria2c -V -d jlink https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_Linux-amd64.tar.gz && sync
RUN cd jlink; tar -xvf nrf-command-line-tools-${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_linux-amd64.tar.gz && dpkg -i JLink_Linux_*.deb; apt-get --fix-broken install -y
RUN rm -rf jlink

RUN aria2c -V -d command-line https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-${TOOLS_VERSION}-x-x/${TOOLS_VERSION}-${TOOLS_VERSION_MAJOR}-${TOOLS_VERSION_MINOR}/nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb && sync

RUN cd command-line; dpkg -i nrf-command-line-tools_${TOOLS_VERSION}.${TOOLS_VERSION_MAJOR}.${TOOLS_VERSION_MINOR}_amd64.deb; apt-get --fix-broken install -y
RUN rm -rf command-line

COPY aria2c_python_requirements.txt aria2c_python_requirements.txt
RUN aria2c -i aria2c_python_requirements.txt && sync

RUN pip3 install -r pip-nrf/requirements.txt 
RUN pip3 install -r pip-zephyr/requirements.txt
RUN pip3 install -r pip-mcuboot/requirements.txt

RUN rm -rf pip-nrf* pip-zephyr* pip-mcuboot*
 
RUN aria2c -V -d zephyr-sdk https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.gz && sync
RUN cd zephyr-sdk; tar -xvf zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.gz && mv zephyr-sdk-${SDK_VERSION} /opt
RUN rm -rf zephyr-sdk
RUN /opt/zephyr-sdk-${SDK_VERSION}/setup.sh -h -c

RUN aria2c  -V -d go_download https://go.dev/dl/go1.19.3.linux-amd64.tar.gz
RUN cd go_download; tar -C /usr/local -xzf go1.19.3.linux-amd64.tar.gz
RUN rm -rf go_download
RUN echo 'export GOPATH=/home/'"$UNAME" >> /home/$UNAME/.profile
RUN echo 'export PATH=$PATH:/usr/local/go/bin:$GOPATH/bin' >> /home/$UNAME/.profile
RUN . /home/$UNAME/.profile; go install github.com/apache/mynewt-mcumgr-cli/mcumgr@latest

RUN echo 'export PATH=$PATH:/home/'"$UNAME"'/.local/bin' >> /home/$UNAME/.bashrc

RUN echo "$UNAME:$UNAME_PASSWORD" | chpasswd 
USER $UNAME
RUN mkdir -p /home/$UNAME/work/ncs

# add your commands below
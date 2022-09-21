FROM ubuntu:22.04

ARG UNAME=vscode
ARG UNAME_PASSWORD=pass

RUN useradd -ms /bin/bash $UNAME 
RUN usermod -aG sudo $UNAME
RUN usermod -aG dialout $UNAME

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get upgrade -y; apt-get install --no-install-recommends -y wget python3-pip git cmake ninja-build gperf  \
  ccache dfu-util device-tree-compiler \
  python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file  \
  make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 udev clangd ssh nano bash-completion sudo gpg ruby lcov screen uncrustify

RUN wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-17-3/nrf-command-line-tools-10.17.3_Linux-amd64.tar.gz && \
tar -xvf nrf-command-line-tools-10.17.3_Linux-amd64.tar.gz
RUN dpkg -i JLink_Linux_V766a_x86_64.deb; apt-get --fix-broken install -y

RUN wget https://www.nordicsemi.com/-/media/Software-and-other-downloads/Desktop-software/nRF-command-line-tools/sw/Versions-10-x-x/10-17-3/nrf-command-line-tools_10.17.3_amd64.deb
RUN dpkg -i nrf-command-line-tools_10.17.3_amd64.deb; apt-get --fix-broken install -y

RUN pip3 install alabaster==0.7.12 anytree==2.8.0 appdirs==1.4.4 arrow==1.2.1 \
astroid==2.12.10 attrs==22.1.0 azure-core==1.25.1 azure-storage-blob==12.13.1 \
Babel==2.10.3 breathe==4.34.0 canopen==2.0.0 capstone==4.0.2 cbor==1.0.0 \
cbor2==5.4.3 certifi==2022.9.14 cffi==1.15.1 charset-normalizer==2.1.1 \
clang-format==14.0.6 click==8.0.3 cmsis-pack-manager==0.4.0 colorama==0.4.5 \
commonmark==0.9.1 coverage==6.4.4 cryptography==38.0.1 Deprecated==1.2.13 \
dill==0.3.5.1 docopt==0.6.2 docutils==0.17.1 ecdsa==0.18.0 future==0.18.2 \
gcovr==5.2 gitlint==0.17.0 gitlint-core==0.17.0 graphviz==0.20.1 grpcio==1.48.1 \
grpcio-tools==1.48.1 idna==3.4 imagesize==1.4.1 imgtool==1.9.0 iniconfig==1.1.1 \
intelhex==2.3.0 intervaltree==3.1.0 isodate==0.6.1 isort==5.10.1 Jinja2==3.0.3 \
junit2html==30.1.3 junitparser==2.8.0 lazy-object-proxy==1.7.1 libusb-package==1.0.26.0 \
lpc-checksum==2.2.0 lxml==4.9.1 m2r2==0.3.2 Markdown==3.4.1 MarkupSafe==2.1.1 \
mccabe==0.7.0 milksnake==0.1.5 mistune==0.8.4 mock==4.0.3 msgpack==1.0.4 \
msrest==0.7.1 mypy==0.971 mypy-extensions==0.4.3 natsort==8.2.0 netifaces==0.11.0 \
oauthlib==3.2.1 packaging==21.3 Pillow==9.2.0 platformdirs==2.5.2 pluggy==1.0.0 \
ply==3.11 prettytable==3.4.1 progress==1.6 protobuf==3.20.2 psutil==5.9.2 \
py==1.11.0 pycparser==2.21 pyelftools==0.29 pygit2==1.10.1 PyGithub==1.55 \
Pygments==2.13.0 PyJWT==2.5.0 pykwalify==1.8.0 pylink-square==0.14.2 \
pylint==2.15.3 PyNaCl==1.5.0 pyocd==0.34.1 pyparsing==3.0.9 pyserial==3.5 \
pytest==7.1.3 python-can==4.0.0 python-dateutil==2.8.2 python-magic==0.4.27 \
pytz==2022.2.1 pyusb==1.2.1 PyYAML==6.0 recommonmark==0.6.0 regex==2022.9.13 \
requests==2.28.1 requests-oauthlib==1.3.1 ruamel.yaml==0.17.21 ruamel.yaml.clib==0.2.6 \
sh==1.14.2 six==1.16.0 snowballstemmer==2.2.0 sortedcontainers==2.4.0 \
Sphinx==4.5.0 sphinx-copybutton==0.5.0 sphinx-markdown-tables==0.0.17 sphinx-ncs-theme==1.0.13 \
sphinx-notfound-page==0.8.3 sphinx-rtd-theme==1.0.0 sphinx-tabs==3.4.0 \
sphinx-togglebutton==0.3.2 sphinxcontrib-applehelp==1.0.2 sphinxcontrib-devhelp==1.0.2 \
sphinxcontrib-htmlhelp==2.0.0 sphinxcontrib-jsmath==1.0.1 sphinxcontrib-mscgen==0.6 \
sphinxcontrib-plantuml==0.24 sphinxcontrib-qthelp==1.0.3 sphinxcontrib-serializinghtml==1.1.5 \
sphinxcontrib-svg2pdfconverter==1.2.0 tabulate==0.8.10 tomli==2.0.1 \
tomlkit==0.11.4 typing_extensions==4.3.0 urllib3==1.26.12 wcwidth==0.2.5 \
west==0.14.0 wrapt==1.14.1 zcbor==0.5.1

RUN wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.14.1/zephyr-sdk-0.14.1_linux-x86_64.tar.gz
RUN tar -xvf zephyr-sdk-0.14.1_linux-x86_64.tar.gz && mv zephyr-sdk-0.14.1 /opt
RUN /opt/zephyr-sdk-0.14.1/setup.sh -h -c

RUN echo 'export PATH=$PATH:/home/'"$UNAME"'/.local/bin' >> /home/$UNAME/.bashrc

RUN echo "$UNAME:$UNAME_PASSWORD"  | chpasswd 
USER $UNAME
RUN mkdir -p /home/$UNAME/work/ncs

FROM nordicplayground/nrfconnect-sdk:v2.3-branch

ARG UNAME=vscode
ARG UNAME_PASSWORD=pass

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
RUN sudo chmod $UNAME /workdir; sudo mkdir -p /work/ncs
WORKDIR /work/ncs

# add your commands below

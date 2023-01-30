#!/bin/bash

sudo apt-get remove -y docker docker-engine docker.io containerd runc

sudo apt-get update
sudo apt-get install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

sudo apt-get update

sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-compose-plugin

sudo bash -c "echo '{
    \"storage-driver\": \"vfs\"
}' > /etc/docker/daemon.json"

sudo groupadd docker
sudo usermod -aG docker vscode

cd /home/vscode/work/ncs/
git clone ssh://git@bitbucket.nordicsemi.no:7999/ncs-test/test-sdk-sidewalk.git
cd test-sdk-sidewalk
git submodule set-url bts/common ssh://git@bitbucket.nordicsemi.no:7999/krknwk/common-verification-modules.git
git submodule init
git submodule update

echo "Provide file \"/home/vscode/TargetTestsDevConf/user_devconf.yml"

echo "example content of this file:"
echo "devices:
  - name: board_0
    pca: PCA10056
    revision: '2.0.2'
    type: 'devkit'
    segger: '683133154'
    cdc_serial: 'EA87EE4A4A23'
    usb_connected: True


  - name: board_1
    pca: PCA10056
    revision: '2.0.1'
    type: 'devkit'
    segger: '683420226'
    cdc_serial: 'FA60CAB0041B'
    usb_connected: False
    semtech_connected: True
    "

echo "

Close This container and reopen it
"

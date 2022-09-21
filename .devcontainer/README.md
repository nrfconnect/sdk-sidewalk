# Developer environment for Sidewalk

This is the developer environment contained inside docker created to work with 
[remote-containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension for VSCode

## Installation

1. On local machine download sidewalk repository 

```bash
cd ~/your/favourite/location/for/projects
git clone git@github.com:nrfconnect/sdk-sidewalk.git
```
2. [Install docker](https://docs.docker.com/engine/install/ubuntu/)
3. Open VScode and install [remote-containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension for VSCode

> **_NOTE:_** Execution of next points may take few minutes on first run

4. Open sdk-sidewalk project in container

```bash
ctrl+shift+p
Remote-Containers: Open Folder in Container
```

> **_NOTE:_**  Your username is `vscode` and password is `pass` This is user inside container, but it is mapped to the same uid and gid as your local user.

5. Run bootstrap script

```bash
bash /home/vscode/work/ncs/sidewalk/.devcontainer/bootstrap.sh
```

## Usage

The environment has already installed few extensions, and it is ready to work.
You can build, flash and debug applications even on the development boards.

If the host user has ssh and/or gpg keys setup, they are available inside container.
If the user is logged into github account the vscode authentication through github also work inside container.

The container can be closed, and all the data inside the container persists, they are purged when the container is rebuild, but the sidewalk directory stays intact because it is stored on host machine and it is only mapped to the container. Therefore repositories downloaded via west, will be purged when the container is rebuild.

## Known Issues

- Do not add anything to git config in "PostCreateCommand" in devcontainer.json - it is known to break the integration of ssh/gpg keys with host machine.

- if the flash operation fails without the use of sudo, please install SEGGER Jlink tools to host machine

- Downloading private repository in "PostCreateCommand" require username and password even if the vscode is integrated with github account - to overcome this issue bootstrap.sh has been implemented

- The older Docker version may have problem with running Ubuntu 22.04, sse [docker/hub-feedback#2235](https://github.com/docker/hub-feedback/issues/2235)


## Contributing
Pull requests are welcome.

Contribution Guidelines
#######################

The Sidewalk repository in general follows the Zephyr RTOS `Contribution Guidelines <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/contribute/index.html>`

This documents presents an overview of Zephyr Contribution Guidelines and differences specific for this repo:

* The Sidewalk repository uses the nRF Connect SDK Nordic License.

* There are some imported or reused components that use other licensing and are clearly identified. Some of them use the proprietary Amazon Sidewalk Program license. The full text of Amazon License is provided in `LICENSE_AMAZON <./LICENSE_AMAZON.txt>`_.

* Sidewalk development is supported on Linux with GNU Arm Embedded Toolchain. For Windows is is advised to use WSL or CygWin. For MacOS it should works, but some additional work might be required.

* It is advised to use `Visual Studio Code IDE <https://code.visualstudio.com>` with the `nRF Connect Extension Pack <https://marketplace.visualstudio.com/items?itemName=nordic-semiconductor.nrf-connect-extension-pack>`_.

* The repository follow the Forking Workflow. Instead of using a single server-side repository to act as the “central” codebase, it gives every developer their own server-side repository. This means that each contributor has not one, but two Git repositories: a private local one and a public server-side one. You can read more about GitHub Forking Workflow contribution in `Quick Start Tutorial <https://docs.github.com/en/get-started/quickstart/contributing-to-projects>`_.

* All changes on the main repository branch should be added with Pull Request. It is required to have at least one review approval to merge, and pass all related CI tests.

* Changes should be added with fast-forward rebase.

* Issue and feature tracking is done in Nordic Sidewalk Team internal Jira.

* TODO: *A Continuous Integration (CI) system runs on every Pull Request (PR) to verify several aspects of the PR including Git commit formatting, Coding Style, sanity checks builds, and documentation builds.*

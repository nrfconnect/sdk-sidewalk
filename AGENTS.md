# AGENTS.md

Agent instructions for the **nRF Connect SDK Amazon Sidewalk** repository.
Applies to Claude Code, GitHub Copilot, Cursor, Codex, and similar AI coding assistants.

## Knowledge Sources

Prefer up-to-date sources over built-in knowledge:

- **nrflow MCP** (if available) — nRF Connect SDK, Zephyr, west (`nordicsemi_search_sources` / `nordicsemi_workflow_ncs`)
- **context7 MCP** (if available) — external library and tool docs: CMake, Python libs, AWS, etc. (`resolve-library-id` + `query-docs`)
- **nordic-semiconductor-docs MCP** (if available) — Nordic Semiconductor product documentation and datasheets
- **Project documentation** — reStructuredText docs in `doc/` (see [Documentation](#documentation) section below)

MCP servers may not be available in all environments; fall back to project docs and web search if needed.

## Project Overview

This is the **nRF Connect SDK Amazon Sidewalk** repository — a Platform Abstraction Layer (PAL) and sample application framework for Amazon Sidewalk on Nordic Semiconductor nRF hardware (nRF52840, nRF5340, nRF54L15). It targets Zephyr RTOS with support for BLE, LoRa, and FSK link types. The Sidewalk protocol core is shipped as prebuilt `.a` libraries.

## Build System

This repository is designed to live inside a west workspace. The workspace root is the **parent directory** of this repo, where `.west/` resides. All commands below assume the workspace root as the working directory; paths like `sidewalk/` refer to this repo from there.

### Workspace initialization check

Before running any build command, verify the workspace is initialized:

```bash
ls ../.west   # must exist; if not, the workspace is not set up
```

If `../.west/` is missing, the workspace has not been initialized. Report this to the user and direct them to the SDK setup guide:
`doc/setting_up_sidewalk_environment/setting_up_sdk.rst`

### Determining the toolchain version

The required NCS version is declared in `west.yml` (this repo) under `projects[name=nrf].revision`:

```bash
grep revision west.yml   # e.g. v3.0.0
```

`west` must be invoked through `nrfutil toolchain-manager` — it is not on PATH directly. Use `--ncs-version` with the version from `west.yml`. Before running any build command:

1. **Check available toolchains:**
   ```bash
   nrfutil toolchain-manager list
   ```
2. **Exact match** — if the version from `west.yml` is listed, use it directly:
   ```bash
   nrfutil toolchain-manager launch --ncs-version v3.0.0 --chdir .. -- west ...
   ```
3. **No exact match** — pick the closest installed version using semver comparison (prefer same major.minor, nearest patch/pre-release). Inform the user which version is being used as a substitute.

4. **No toolchain installed** — report to the user and ask them to install the required toolchain:
   ```bash
   nrfutil toolchain-manager install --ncs-version v3.0.0
   ```

## Common Commands

All commands run from the **workspace root** (parent directory of this repo). Replace `<ncs-version>` with the version resolved above.

### Build a sample

```bash
# Default: hello app, BLE link, nrf54l15dk/nrf54l10/cpuapp
nrfutil toolchain-manager launch --ncs-version <ncs-version> --chdir .. -- \
  west build -b nrf54l15dk/nrf54l10/cpuapp sidewalk/samples/sid_end_device --sysbuild
```

Artifacts: `build/zephyr/zephyr.elf`, `build/merged.hex`, `build/dfu_application.zip`.

For LoRa/FSK, add `-DOVERLAY_CONFIG=overlay-hello.conf` and the appropriate shield:
```bash
  -DSHIELD="simple_arduino_adapter;semtech_sx1262mb2cas"
```

### Flash

```bash
nrfutil toolchain-manager launch --ncs-version <ncs-version> --chdir .. -- \
  west flash
```

## Documentation

The project documentation is written in **reStructuredText** and lives in the `doc/` directory. Read it to understand how to build, configure, run, and test Sidewalk — do not duplicate it here (DRY).

**How to navigate:**

- Start at `doc/index.rst` — it contains the root `toctree` linking all top-level sections.
- Each `.rst` file may contain a nested `toctree` pointing to further sub-pages; follow the tree recursively to find relevant content.
- Key top-level sections and their entry points:
  - `doc/setting_up_sidewalk_environment/setting_up_environment.rst` — environment setup, hardware requirements, SDK installation, prototype setup, manufacturing environment
  - `doc/samples/samples_list.rst` — sample overview and board/variant matrix
  - `doc/testing.rst` — testing instructions
  - `doc/compatibility_matrix.rst` — supported hardware and NCS version compatibility
  - `doc/release_notes_and_migration.rst` — release notes and migration guides
  - `doc/known_issues.rst` — known issues
  - `doc/additional_resources.rst` — external links and references

When asked about setup, building, flashing, or testing, read the relevant `.rst` file(s) before answering.

## Architecture

### Layer Stack

```
Application (samples/sid_end_device/src/{hello,sensor_monitoring,cli,sbdt})
    ↓
Sidewalk Thread + Message Queue (sidewalk.c / sidewalk_events.c)
    ↓
Sidewalk Protocol Core (prebuilt libs: lib/cortex-m4/ or lib/cortex-m33/)
    ↓
PAL — Platform Abstraction Layer (subsys/sal/sid_pal/)
    ↓
Zephyr drivers + Semtech radio drivers (subsys/semtech/)
    ↓
Hardware (nRF SoC)
```

### Key Directories

- **`lib/`** — Prebuilt Sidewalk protocol libraries. Two architecture variants (`cortex-m4`, `cortex-m33`), each with `ble_only` and `lora_fsk` sub-variants.
- **`subsys/sal/`** — PAL implementations for BLE adapter, radio, crypto (PSA-based), storage, timers, serial bus. The `common/` subdirectory contains interface headers; `sid_pal/` contains Zephyr-specific implementations.
- **`subsys/hal/`** — Minimal HAL for memory and reset.
- **`subsys/ace/`** — AWS Cloud for Edge OSAL — OS abstraction for memory allocation used by the Sidewalk library.
- **`subsys/semtech/`** — Third-party SX126x, LR1110, LR11xx radio drivers.
- **`samples/sid_end_device/`** — The primary sample application. `main.c` → `app_start()` → dedicated Zephyr thread with a `K_MSGQ`-backed event queue.
- **`utils/`** — Reusable utilities: TLV encoding, settings/NVS helpers, button handling, state notifier (LED/log backends), nanopb (protobuf), DFU support.
- **`tests/unit_tests/`** — 20+ x86 unit test suites covering BLE callbacks, PAL (timer, temp, logging), TLV, manufacturing parsers, message queuing, etc.

### Event-Driven Design

The sample application uses a dedicated Zephyr thread (`sid_thread_entry`) with a kernel message queue. Sidewalk library callbacks post events to this queue; the thread dispatches them to handlers defined in `sidewalk_events.c`. Core event types: `process`, `platform_init`, `connect`, `autostart`, `send_msg`, `factory_reset`.

### Configuration

- **Kconfig** (`Kconfig`, `Kconfig.dependencies`) — compile-time feature selection: link type (BLE/LoRa/FSK), heap size, thread stack/priority, logging verbosity.
- **Device tree overlays** (`dts/`) — board-specific hardware configuration.
- **`test_config.yaml`** — Twister test levels: `samples`, `unit`, `integration`, `Github_tests`, `bugs`.

### Manufacturing Data

Manufacturing credentials are provisioned per-device using the tools in `tools/provision/`. The flow requires a Sidewalk account and device JSON files obtained from AWS IoT (either a combined `certificate.json` or separate `wireless_device.json` + `device_profile.json`).

Provisioning is board-specific — the `--addr` argument and flash command differ per target:

| Target | `--addr` | Flash command |
|--------|----------|---------------|
| nRF52840 / nRF5340 DKs | `0xFF000` | `nrfjprog --sectorerase --program <file>.hex --reset` |
| nRF54L10 (emulated on nRF54L15 DK) | `0xFF000` | `nrfutil device program --x-family nrf54l ...` |
| nRF54L15 DK | `0x17c000` | `nrfutil device program --x-family nrf54l ...` |

Example for nRF54L15:
```bash
python3 tools/provision/provision.py nordic aws \
  --output_bin mfg.bin \
  --certificate_json certificate.json \
  --addr 0x17c000 \
  --output_hex nordic_aws_nrf54l15.hex

nrfutil device program \
  --x-family nrf54l \
  --options chip_erase_mode=ERASE_RANGES_TOUCHED_BY_FIRMWARE,reset=RESET_PIN,verify=VERIFY_READ \
  --traits jlink \
  --firmware nordic_aws_nrf54l15.hex
```

Full provisioning instructions (including onboarding and MQTT destination setup) are in:
`doc/setting_up_sidewalk_environment/setting_up_sidewalk_prototype.rst`

The TLV-based manufacturing hex format (v7 and v8) is handled by parsers in `utils/` and tested in `tests/unit_tests/mfg_parsers/`.

## CI

GitHub workflows in `.github/workflows/`:
- `run_tests.yml` — x86 unit tests with coverage
- `samples_build.yml` — multi-board sample build matrix
- `validate_code_style.yml` — formatting, licensing, compliance
- `on-pr.yml` / `on-commit.yml` — triggered automatically on PRs and commits

### Running CI checks locally

The CI workflows invoke scripts from this repo and from `zephyr/`. All commands below run from the **workspace root**. Replace `<ncs-version>` with the version resolved from `west.yml`.

#### Compliance check (yamllint, kconfig, devicetree, gitlint, …)

Requires `zephyr/zephyr-env.sh` to be sourced and the compliance pip dependencies installed:

```bash
pip3 install -r ../zephyr/scripts/requirements-compliance.txt
source ../zephyr/zephyr-env.sh
cd sidewalk
python3 scripts/ci/sid_compliance.py --annotate \
  -m yamllint -m kconfigbasic -m devicetreebindings \
  -m binaryfiles -m imagesize -m nits -m gitlint -m identity \
  -c origin/main..HEAD
```

`-c <base>..HEAD` sets the commit range to check. Use `origin/main..HEAD` for a local branch, or pass a specific commit SHA.

#### Formatting check

```bash
pip3 install -r sidewalk/scripts/ci/requirements.txt
python3 sidewalk/scripts/ci/verify_formatting.py -d -s sidewalk -c sidewalk/scripts/ci/formatter_cfg.yml
```

#### License check

```bash
python3 sidewalk/scripts/ci/verify_license.py -s sidewalk -c sidewalk/scripts/ci/license.yml
```

#### Unit tests (x86, no hardware needed)

```bash
source ../zephyr/zephyr-env.sh
nrfutil toolchain-manager launch --ncs-version <ncs-version> --chdir .. -- \
  west twister --test-config sidewalk/test_config.yaml \
    --level="Github_tests" --testsuite-root sidewalk \
    --platform native_sim --platform unit_testing \
    --coverage --coverage-basedir sidewalk --coverage-formats html,xml \
    --enable-ubsan --enable-lsan --enable-asan --inline-logs \
    --overflow-as-errors -vvv
```

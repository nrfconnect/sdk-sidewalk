# Sidewalk Provisioning Script

This script can be used to create a 'manufacturing page' in flash which can be
flashed on the device with the correct Sidewalk identity information and keys.

A device with correctly setup is considered to be provisioned. After
provisioning there are further steps required to register the device on the
Sidewalk network.

## Script setup

```sh
pip3 install --user -r requirements.txt
```

## Script formatting

The script arguments have the following format

```sh
provision.py <PLATFORM_NAME> <INPUT_TYPE> arguments
```

Currently provision script supports 3 platforms

 * _silabs_
 * _nordic_
 * _ti_

and supports 3 input_types

 * _aws_
 * _acs_console_
 * _bb_

_acs_ and _bb_ are on path to be deprecated

Depending on the *PLATFORM_NAME* and *INPUT_TYPE*, the arguments required will differ since each platform supports different types of outputs and miscellaneous input options

## Create manufacturing page by ACS console

Information Required

- Acs Console JSON file download
- Application server public key, which is generated from the
  application_server_cert/generate_application_server.py script (see
  application_server_cert/readme.md)
- SIDEWALK_ID which is embedded in the name of the json file download from ACS
  eg certificate_0123456789.json SIDEWALK_ID=0123456789

### For Nordic devices

```sh
SIDEWALK_ID=0123456789
./provision.py nordic acs --json certificate_${SIDEWALK_ID}.json \
 --app_srv_pub app-server-ed25519.public.bin

Generated <PATH_TO_FOLDER>/nordic_acs_nrf52840.bin
Generated <PATH_TO_FOLDER>/nordic_acs_nrf52840.hex
```

### For TI devices

```sh
SIDEWALK_ID=0123456789
./provision.py ti acs --json certificate_${SIDEWALK_ID}.json \
 --app_srv_pub app-server-ed25519.public.bin

Generated <PATH_TO_FOLDER>/ti_acs_p7.bin
Generated <PATH_TO_FOLDER>/ti_acs_p7.hex
```

### For Silabs devices (xG21 or xG24)

In order to provision a Silabs device (EFR32xG21 or EFR32xG24). For the script to run properly, Simplicity Commander utility (that comes in the same bundle with [Simplicity Studio](https://www.silabs.com/developers/simplicity-studio)), that performs some intermediary steps (specific to Silabs devices) for the manufacturing page generation, must be installed locally and its path must be exported as below.

> **Ⓘ INFO Ⓘ**: If you run this script inside WSL, note that some adaptations are needed to call Windows executables from WSL Linux. The parameter `--commander-bin` is available to manually add commander path and `.exe` extension.

- First, add the Simplicity Commander tool in your path (the default path is *[Simplicity Studio Path]/developer/adapter_packs/commander/).
- Alternatively, if you do not wish to add the Simplicity Commander to your PATH variable, you can add it as an argument as follows: `--commander-bin /path/to/simplicity/commander/commander.exe`.
- Call *provision.py* script with your device series (chip), memory footprint, your json certificate and the *app-server-ed25519.public.bin* file.

Device series can be any of the following:

- mg21 for EFR32MG21B020F1024IM32 (BRD4181C)
- bg21 for EFR32BG21B020F1024IM32 (KG100S)
- mg24 for EFR32MG24BA020F1536GM48 (BRD4187C)
- bg24 for EFR32BG24BA020F1536GM48 (BRD4187C)

Memory footprint can be 512, 768, 1024 or 1536 depending on your radio board (flash memory size).

You can optionally use '--secure-vault' or '-sv' if you use secure vault.

Example of using the provision.py script:

```sh
SIDEWALK_ID=0123456789
python3 ./provision.py silabs acs --chip mg21 --memory 1024 --json certificate_${SIDEWALK_ID}.json --app_srv_pub app-server-ed25519.public.bin
```

### The app_srv_pub key can also be given as a 32 byte hex string

```
APP_SRV_PUB_HEX=0123456789012345678901234567890123456789012345678901234567890123
SIDEWALK_ID=0123456789
./provision.py acs --json certificate_${SIDEWALK_ID}.json \
 --app_srv_pub ${APP_SRV_PUB_HEX} \
 --config ${MFG_PAGE_CONFIG} --output_bin mfg.bin
```

## Create manufacturing page by AWS cli

Information Required

- JSON response of `aws iotwireless get-device-profile .... > device_profile.json` response saved to device_profile.json
- JSON response of `aws iotwireless get-wireless-device .... > wiresless_device.json` response saved to wireless_device.json

### For Nordic devices

```sh
SIDEWALK_ID=0123456789
./provision.py nordic aws --wireless_device_json wireless_device.json \
  --device_profile_json device_profile.json

Generated <PATH_TO_FOLDER>/nordic_aws_nrf52840.bin
Generated <PATH_TO_FOLDER>/nordic_aws_nrf52840.hex
```

### For Ti devices

```sh
SIDEWALK_ID=0123456789
./provision.py nordic aws --wireless_device_json wireless_device.json \
  --device_profile_json device_profile.json

Generated <PATH_TO_FOLDER>/ti_aws_P7.bin
Generated <PATH_TO_FOLDER>/ti_aws_P7.hex
```

### For Silabs devices (xG21 or xG24)

In order to provision a Silabs device (EFR32xG21 or EFR32xG24). For the script to run properly, Simplicity Commander utility (that comes in the same bundle with Simplicity Studio), that performs some intermediary steps (specific to Silabs devices) for the manufacturing page generation, must be installed locally and its path must be exported as below.

Ⓘ INFO Ⓘ: If you run this script inside WSL, note that some adaptations are needed to call Windows executables from WSL Linux. The parameter --commander-bin is available to manually add commander path and .exe extension.

First, add the Simplicity Commander tool in your path (the default path is *[Simplicity Studio Path]/developer/adapter_packs/commander/).
Alternatively, if you do not wish to add the Simplicity Commander to your PATH variable, you can add it as an argument as follows: --commander-bin /path/to/simplicity/commander/commander.exe.
Call provision.py script with your device series (chip), memory footprint, your json certificate and the app-server-ed25519.public.bin file.
Device series can be any of the following:

mg21 for EFR32MG21B020F1024IM32 (BRD4181C)
bg21 for EFR32BG21B020F1024IM32 (KG100S)
mg24 for EFR32MG24BA020F1536GM48 (BRD4187C)
bg24 for EFR32BG24BA020F1536GM48 (BRD4187C)
Memory footprint can be 512, 768, 1024 or 1536 depending on your radio board (flash memory size).

You can optionally use '--secure-vault' or '-sv' if you use secure vault.

Example of using the provision.py script:

```sh
SIDEWALK_ID=0123456789
./provision.py silabs aws --chip mg21 --memory 1024 --wireless_device_json wireless_device.json --device_profile_json device_profile.json
```

Upon success, a `silabs_aws_[series].s37` file containing the device manufacturing page will be created. (Or `silabs_aws_[series]_sv.s37` in case of a secure vault.)

# Create manufacturing page by Black Box JSON

Used if connecting to a Black Box server to register devices during manufacturing

Information Required

- JSON response from the Black Box server, saved to a file
- Application server public key, which is generated from the
  application_server_cert/generate_application_server.py script (see
  application_server_cert/readme.md)

```
./provision.py bb --config ${MFG_PAGE_CONFIG} --json bb_response.json \
        --output_bin mfg.bin \
        --app_srv_pub app-server-ed25519.public.bin
```

---
**NOTE**

For platforms such that _ti_ and _nordic_ if the user gives the `output_bin` argument and  the bin file indicated by `output_bin` contains prefilled binary, then only the
data offsets indicated by platform `config.yaml` are overwritten, the rest of the binary
file is left as is. This allows for users to merge sidewalk provision data and
their own custom mfg data.

To see the default `config.yaml` for the platform run the script with help argument

```sh
./provision.py nordic aws -h
usage: ./provision.py nordic aws [-h] [--wireless_device_json WIRELESS_DEVICE_JSON] [--device_profile_json DEVICE_PROFILE_JSON]
                                 [--certificate_json CERTIFICATE_JSON] [--chip {nrf52840}] [--dump_raw_values] [--config CONFIG]
                                 [--output_bin OUTPUT_BIN] [--output_hex OUTPUT_HEX]

options:
  -h, --help            show this help message and exit
  --wireless_device_json WIRELESS_DEVICE_JSON
                        Json Response of 'aws iotwireless get-wireless-device'
  --device_profile_json DEVICE_PROFILE_JSON
                        Json response of 'aws iotwireless get-device-profile ...'
  --certificate_json CERTIFICATE_JSON
                        Certificate json generated from sidewalk aws console
  --chip {nrf52840}     Which chip to generate the mfg page (default: nrf52840)
  --dump_raw_values     Dump the raw values for debugging
  --config CONFIG       Config Yaml that defines the mfg page offsets (default: <PATH_TO_CONFIG_FILE>)
  --output_bin OUTPUT_BIN
                        Output bin file, if this file does not exist - it will be created, if it does exist the data at - the offsets
                        defined in the config file will be - overwritten by provision data (default: <PATH_TO_OUTPUT_BIN>)
  --output_hex OUTPUT_HEX
                        Output hex file, default chip offset is used when generating hexfile  (default: <PATH_TO_OUTPUT_HEX>)
```

The default config file is indicated by --config CONFIG    < Help> default: <PATH TO FILE>

---

## Flash Nordic-DK board with nrfproj

 List serial number of DK board with nrfjprog, set the value of JPROG_SERIALNO appropriately

```
$ nrfjprog -i
683416416
$ JPROG_SERIALNO=683416416
$ nrfjprog --program nordic_aws_nrf52840.hex --sectorerase --reset --snr ${JPROG_SERIALNO}

Parsing hex file.
Erasing page at address 0xFD000.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
Applying system reset.
Run.
```

## Flash manufacturing page with Simplicity Commander

To set commander path:

```sh
COMMANDER_BIN_PATH=/path/to/simplicity/commander
```

To identify connected JLink device serial:

```sh
JLinkExe
> showemulist
J-Link[0]: Connection: USB, Serial number: 440130584, ProductName: J-Link EnergyMicro

JPROG_SERIALNO=440130584
```

`silabs_aws_[series].s37` manufacturing page must have been generated previously by executing the provisioning script for Silabs devices mentioned in above sections.

__xg21__

```sh
$(COMMANDER_BIN_PATH)/commander flash silabs_aws_[series].s37 --address 0x000F2000 --serialno ${JPROG_SERIALNO}
```

__xg24__

```sh
$(COMMANDER_BIN_PATH)/commander flash silabs_aws_[series].s37 --address 0x08172000 --serialno ${JPROG_SERIALNO}
```

# Running unit tests

```
python3 -m unittest discover tests
```

# Sidewalk Provisioning Script

This script can be used to create a 'manufacturing page' in flash which can be
flashed on the device with the correct Sidewalk identity information and keys.

A device with correctly setup is considered to be provisioned. After
provisioning there are further steps required to register the device on the
Sidewalk network.

# Script setup
```
pip3 install --user -r requirements.txt
```

# Select the correct manufacturing page configuration

This step is required for Nordic and TI devices. It can be omitted for Silabs devices.

We need to make sure that the correct manufacturing page is created and matches
the behavior of the corresponding firmware application build.

For Sidewalk/BLE SDK builds the following is recommended:

```
export MFG_PAGE_CONFIG=config.yaml
```

# Create manufacturing page by ACS console

Information Required
- Acs Console JSON file download
- Application server public key, which is generated from the
  application_server_cert/generate_application_server.py script (see
  application_server_cert/readme.md)
- SIDEWALK_ID which is embedded in the name of the json file download from ACS
  eg certificate_0123456789.json SIDEWALK_ID=0123456789


```
SIDEWALK_ID=0123456789
./provision.py acs --json certificate_${SIDEWALK_ID}.json \
	--app_srv_pub app-server-ed25519.public.bin \
	--config ${MFG_PAGE_CONFIG} --output_bin mfg.bin
```

## The app_srv_pub key can also be given as a 32 byte hex string

```
APP_SRV_PUB_HEX=0123456789012345678901234567890123456789012345678901234567890123
SIDEWALK_ID=0123456789
./provision.py acs --json certificate_${SIDEWALK_ID}.json \
	--app_srv_pub ${APP_SRV_PUB_HEX} \
	--config ${MFG_PAGE_CONFIG} --output_bin mfg.bin
```

# Create manufacturing page by AWS cli

Information Required
- JSON response of `aws iotwireless get-device-profile .... > device_profile.json` response saved to device_profile.json
- JSON response of `aws iotwireless get-wireless-device .... > wiresless_device.json` response saved to wireless_device.json


```
./provision.py aws --wireless_device_json wireless_device.json \
  --device_profile_json device_profile.json \
  --config ${MFG_PAGE_CONFIG} --output_bin mfg.bin
```

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

If the bin file indicated by `output_bin` contains prefilled binary, then only the
data offsets indicated by config.yaml are overwritten, the rest of the binary
file is left as is. This allows for users to merge sidewalk provision data and
their own custom mfg data.

---

# Flash Nordic-DK board with nrfproj

# GNU_INSTALL_ROOT will point to the location of the toolchain used to build the SDK
```
export GNU_INSTALL_ROOT=~/sidewalk_mcu/gcc-arm-none-eabi-8-2018-q4-major/bin/
```
# 0xFD000 is the start address of the hex
```
${GNU_INSTALL_ROOT}/arm-none-eabi-objcopy -I binary -O ihex --change-addresses 0xFD000 mfg.bin mfg.hex

$ ls
mfg.bin  mfg.hex
```

# List serial number of DK board with nrfjprog, set the value of JPROG_SERIALNO appropriately
```
$ nrfjprog -i
683416416
$ JPROG_SERIALNO=683416416
$ nrfjprog --program mfg.hex --sectorerase --reset --snr ${JPROG_SERIALNO}

Parsing hex file.
Erasing page at address 0xFD000.
Applying system reset.
Checking that the area to write is not protected.
Programming device.
Applying system reset.
Run.
```

# Running unit tests
```
python3 -m unittest discover tests
```

#!/usr/bin/env python3
#
# Copyright 2020-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file. This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

import argparse
import binascii
import json
import base64
import sys
from ctypes import Structure, c_ubyte
import traceback
import yaml
from prodict import Prodict
from enum import Enum
from pathlib import Path


SMSN_SIZE = 32
SERIAL_SIZE = 4
PRK_SIZE = 32
ED25519_PUB_SIZE = 32
P256R1_PUB_SIZE = 64
SIG_SIZE = 64

# pylint: disable=C0114,C0115,C0116


class SidMfgValueId(Enum):
    """
    Please note that these values have to be in sync at alls times with
    projects/sid/sal/common/public/sid_pal_ifc/mfg_store/sid_pal_mfg_store_ifc.h
    sid_pal_mfg_store_value_t
    """

    SID_PAL_MFG_STORE_MAGIC = 0
    SID_PAL_MFG_STORE_DEVID = 1
    SID_PAL_MFG_STORE_VERSION = 2
    SID_PAL_MFG_STORE_SERIAL_NUM = 3
    SID_PAL_MFG_STORE_SMSN = 4
    SID_PAL_MFG_STORE_APP_PUB_ED25519 = 5
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519 = 6
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519 = 7
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE = 8
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1 = 9
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1 = 10
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE = 11
    SID_PAL_MFG_STORE_DAK_PUB_ED25519 = 12
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE = 13
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL = 14
    SID_PAL_MFG_STORE_DAK_PUB_P256R1 = 15
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE = 16
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL = 17
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519 = 18
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE = 19
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL = 20
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1 = 21
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE = 22
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL = 23
    SID_PAL_MFG_STORE_MAN_PUB_ED25519 = 24
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE = 25
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL = 26
    SID_PAL_MFG_STORE_MAN_PUB_P256R1 = 27
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE = 28
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL = 29
    SID_PAL_MFG_STORE_SW_PUB_ED25519 = 30
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE = 31
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL = 32
    SID_PAL_MFG_STORE_SW_PUB_P256R1 = 33
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE = 34
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL = 35
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519 = 36
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1 = 37
    SID_PAL_MFG_STORE_APID = 38
    SID_PAL_MFG_STORE_CORE_VALUE_MAX = 4000


class StructureHelper:
    def __repr__(self):
        repr_str = "{}\n".format(self.__class__.__name__)
        repr_str += " {}-{}\n".format("device_prk", binascii.hexlify(self.device_prk))
        for _ in self.__class__._fields_:
            field_name = _[0]
            repr_str += " {}-{}\n".format(field_name, binascii.hexlify(getattr(self, field_name)))
        return repr_str


class SidCertMfgAcsP256R1(Structure, StructureHelper):

    # pylint: disable=C0326
    _fields_ = [
        ("smsn", c_ubyte * SMSN_SIZE),
        ("device_pub", c_ubyte * P256R1_PUB_SIZE),
        ("device_sig", c_ubyte * SIG_SIZE),
        ("dak_serial", c_ubyte * SERIAL_SIZE),
        ("dak_pub", c_ubyte * P256R1_PUB_SIZE),
        ("dak_sig", c_ubyte * SIG_SIZE),
        ("product_serial", c_ubyte * SERIAL_SIZE),
        ("product_pub", c_ubyte * P256R1_PUB_SIZE),
        ("product_sig", c_ubyte * SIG_SIZE),
        ("man_serial", c_ubyte * SERIAL_SIZE),
        ("man_pub", c_ubyte * P256R1_PUB_SIZE),
        ("man_sig", c_ubyte * SIG_SIZE),
        ("sw_serial", c_ubyte * SERIAL_SIZE),
        ("sw_pub", c_ubyte * P256R1_PUB_SIZE),
        ("sw_sig", c_ubyte * SIG_SIZE),
        ("root_serial", c_ubyte * SERIAL_SIZE),
        ("root_pub", c_ubyte * P256R1_PUB_SIZE),
        ("root_sig", c_ubyte * SIG_SIZE),
    ]

    def __new__(cls, cert_buffer, priv):
        return cls.from_buffer_copy(cert_buffer)

    def __init__(self, cert_buffer, priv):
        self._cert_buffer = cert_buffer
        self.device_prk = binascii.unhexlify(priv)
        assert len(self.device_prk) == PRK_SIZE, "Invalid P256R1 private key size -{} Expected Size -{}".format(
            len(self.device_prk), PRK_SIZE
        )


class SidCertMfgAcsED25519(Structure, StructureHelper):

    # pylint: disable=C0326
    _fields_ = [
        ("smsn", c_ubyte * SMSN_SIZE),
        ("device_pub", c_ubyte * ED25519_PUB_SIZE),
        ("device_sig", c_ubyte * SIG_SIZE),
        ("dak_serial", c_ubyte * SERIAL_SIZE),
        ("dak_pub", c_ubyte * ED25519_PUB_SIZE),
        ("dak_sig", c_ubyte * SIG_SIZE),
        ("product_serial", c_ubyte * SERIAL_SIZE),
        ("product_pub", c_ubyte * ED25519_PUB_SIZE),
        ("product_sig", c_ubyte * SIG_SIZE),
        ("man_serial", c_ubyte * SERIAL_SIZE),
        ("man_pub", c_ubyte * ED25519_PUB_SIZE),
        ("man_sig", c_ubyte * SIG_SIZE),
        ("sw_serial", c_ubyte * SERIAL_SIZE),
        ("sw_pub", c_ubyte * ED25519_PUB_SIZE),
        ("sw_sig", c_ubyte * SIG_SIZE),
        ("root_serial", c_ubyte * SERIAL_SIZE),
        ("root_pub", c_ubyte * ED25519_PUB_SIZE),
        ("root_sig", c_ubyte * SIG_SIZE),
    ]

    def __new__(cls, cert_buffer, priv):
        return cls.from_buffer_copy(cert_buffer)

    def __init__(self, cert_buffer, priv):
        self._cert_buffer = cert_buffer
        self.device_prk = binascii.unhexlify(priv)
        assert len(self.device_prk) == PRK_SIZE, "Invalid ED25519 private key size -{} Expected Size -{}".format(
            len(self.device_prk), PRK_SIZE
        )


class SidCertMfgAcsCert:
    @staticmethod
    def from_base64(cert, priv, is_p256r1=False):
        if is_p256r1:
            return SidCertMfgAcsP256R1(base64.b64decode(cert), priv)
        return SidCertMfgAcsED25519(base64.b64decode(cert), priv)


class SidMfgObj:
    def __init__(self, name, value, info, id_val, skip=False, word_size=4, is_network_order=True):
        assert name
        assert isinstance(word_size, int)
        assert (word_size > 0 and info) or (word_size == 0 and not info)
        assert isinstance(id_val, int)

        info = Prodict.from_dict(info) if isinstance(info, dict) else info

        self._name = name
        self._value = value
        self._start = 0 if not info else info.start
        self._end = 0 if not info else info.end
        self._word_size = word_size
        self._id_val = id_val
        self._skip = skip

        if info:
            assert self._start < self._end, "Invalid {}  end offset: {} < start offset: {}".format(
                self._name, self._end, self._start
            )
            byte_len = self.end - self.start
        else:
            byte_len = len(value)

        if isinstance(self._value, int):
            self._encoded = (self._value).to_bytes(byte_len, byteorder="big" if is_network_order else "little")
        elif isinstance(self._value, bytes):
            self._encoded = self._value
        elif isinstance(self._value, bytearray):
            self._encoded = bytes(self._value)
        elif isinstance(self._value, str):
            self._encoded = bytes(self._value, "ascii")
        else:
            try:
                self._encoded = bytes(self._value)
            except TypeError as ex:
                raise ValueError("{} Cannot convert value {} to bytes".format(self._name, self._value)) from ex

        if len(self._encoded) < byte_len:
            self._encoded = self._encoded.ljust(byte_len, b"\x00")

        if len(self._encoded) != byte_len:
            ex_str = "Field {} value {} len {} mismatch expected field value len {}".format(
                self._name, self._value, len(self._encoded), byte_len
            )
            raise ValueError(ex_str)

    @property
    def name(self):
        return self._name

    @property
    def start(self):
        return self._start * self._word_size

    @property
    def end(self):
        return self._end * self._word_size

    @property
    def encoded(self):
        return self._encoded

    @property
    def id_val(self):
        return self._id_val

    @property
    def skip(self):
        return self._skip

    def __repr__(self):
        return "{}[{}:{}] - {}".format(self._name, self.start, self.end, binascii.hexlify(self._encoded))


class SidMfg:
    def __init__(self, app_pub, config, is_network_order):
        self._config = config
        self._app_pub = app_pub
        self._p256r1 = None
        self._ed25519 = None
        self._apid = None
        self._is_network_order = is_network_order
        self._mfg_objs = []
        self._word_size = 0 if not self._config else self._config.offset_size

    def __iter__(self):
        return iter(sorted(self._mfg_objs, key=lambda mfg_obj: mfg_obj.id_val))

    def append(self, name, value, can_skip=False):
        try:
            offset_config = 0 if not self._config else self._config.mfg_offsets[name]
            mfg_obj = SidMfgObj(
                name,
                value,
                offset_config,
                id_val=SidMfgValueId[name].value,
                skip=can_skip,
                word_size=self._word_size,
                is_network_order=self._is_network_order,
            )
            self._mfg_objs.append(mfg_obj)
        except KeyError as ex:
            if can_skip:
                print("Skipping {}".format(name))
            else:
                raise ex
        except Exception:
            traceback.print_exc()
            exit(1)


class SidMfgBBJson(SidMfg):
    def __init__(self, bb_json, app_pub, config, is_network_order=True):
        super().__init__(app_pub=app_pub, config=config, is_network_order=is_network_order)

        _bb_json = Prodict.from_dict(bb_json)

        self.append("SID_PAL_MFG_STORE_MAGIC", "SID0", can_skip=True)
        if self._config:
            self.append("SID_PAL_MFG_STORE_VERSION", self._config.mfg_page_version, can_skip=True)
        self.append("SID_PAL_MFG_STORE_DEVID", binascii.unhexlify(_bb_json.ringNetDevId))
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519",
            binascii.unhexlify(_bb_json.PKI.device_cert.ed25519_priv),
        )
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PUB_ED25519",
            binascii.unhexlify(_bb_json.PKI.device_cert.ed25519_pub),
        )
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE",
            binascii.unhexlify(_bb_json.PKI.device_cert.ed25519_signature),
        )
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1",
            binascii.unhexlify(_bb_json.PKI.device_cert.p256r1_priv),
        )
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PUB_P256R1",
            binascii.unhexlify(_bb_json.PKI.device_cert.p256r1_pub),
        )
        self.append(
            "SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE",
            binascii.unhexlify(_bb_json.PKI.device_cert.p256r1_signature),
        )

        for cert in _bb_json.PKI.intermediate_certs:
            _cert = Prodict.from_dict(cert)
            if _cert.cert_name == "AMZN":
                self.append(
                    "SID_PAL_MFG_STORE_AMZN_PUB_ED25519",
                    binascii.unhexlify(_cert.ed25519_pub),
                )
                self.append(
                    "SID_PAL_MFG_STORE_AMZN_PUB_P256R1",
                    binascii.unhexlify(_cert.p256r1_pub),
                )
            elif _cert.cert_name == "MAN":
                self.append(
                    "SID_PAL_MFG_STORE_MAN_PUB_ED25519",
                    binascii.unhexlify(_cert.ed25519_pub),
                )
                self.append(
                    "SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE",
                    binascii.unhexlify(_cert.ed25519_signature),
                )
                self.append(
                    "SID_PAL_MFG_STORE_MAN_ED25519_SERIAL",
                    binascii.unhexlify(_cert.ed25519_serial),
                )
                self.append(
                    "SID_PAL_MFG_STORE_MAN_PUB_P256R1",
                    binascii.unhexlify(_cert.p256r1_pub),
                )
                self.append(
                    "SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE",
                    binascii.unhexlify(_cert.p256r1_signature),
                )
                self.append(
                    "SID_PAL_MFG_STORE_MAN_P256R1_SERIAL",
                    binascii.unhexlify(_cert.p256r1_serial),
                )
            elif _cert.cert_name == "MODEL":
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519",
                    binascii.unhexlify(_cert.ed25519_pub),
                )
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE",
                    binascii.unhexlify(_cert.ed25519_signature),
                )
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL",
                    binascii.unhexlify(_cert.ed25519_serial),
                )
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1",
                    binascii.unhexlify(_cert.p256r1_pub),
                )
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE",
                    binascii.unhexlify(_cert.p256r1_signature),
                )
                self.append(
                    "SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL",
                    binascii.unhexlify(_cert.p256r1_serial),
                )


class SidMfgAcsJson(SidMfg):
    def __init__(self, acs_json, app_pub, config, is_network_order=True):
        super().__init__(app_pub=app_pub, config=config, is_network_order=is_network_order)

        _acs_json = Prodict.from_dict(acs_json)
        self._ed25519 = SidCertMfgAcsCert.from_base64(_acs_json.eD25519, _acs_json.metadata.devicePrivKeyEd25519)
        self._p256r1 = SidCertMfgAcsCert.from_base64(
            _acs_json.p256R1, _acs_json.metadata.devicePrivKeyP256R1, is_p256r1=True
        )
        self._apid = _acs_json.metadata.apid
        self._smsn = binascii.unhexlify(_acs_json.metadata.smsn)

        self.append("SID_PAL_MFG_STORE_MAGIC", "SID0", can_skip=True)
        if self._config:
            self.append("SID_PAL_MFG_STORE_VERSION", self._config.mfg_page_version, can_skip=True)
        self.append("SID_PAL_MFG_STORE_SMSN", self._smsn)
        self.append("SID_PAL_MFG_STORE_APID", self._apid)
        self.append("SID_PAL_MFG_STORE_APP_PUB_ED25519", self._app_pub)
        self.append("SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519", self._ed25519.device_prk)
        self.append("SID_PAL_MFG_STORE_DEVICE_PUB_ED25519", self._ed25519.device_pub)
        self.append("SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE", self._ed25519.device_sig)
        self.append("SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1", self._p256r1.device_prk)
        self.append("SID_PAL_MFG_STORE_DEVICE_PUB_P256R1", self._p256r1.device_pub)
        self.append("SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE", self._p256r1.device_sig)

        self.append("SID_PAL_MFG_STORE_DAK_PUB_ED25519", self._ed25519.dak_pub)
        self.append("SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE", self._ed25519.dak_sig)
        self.append("SID_PAL_MFG_STORE_DAK_ED25519_SERIAL", self._ed25519.dak_serial)
        self.append("SID_PAL_MFG_STORE_DAK_PUB_P256R1", self._p256r1.dak_pub)
        self.append("SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE", self._p256r1.dak_sig)
        self.append("SID_PAL_MFG_STORE_DAK_P256R1_SERIAL", self._p256r1.dak_serial)

        self.append("SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519", self._ed25519.product_pub)
        self.append("SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE", self._ed25519.product_sig)
        self.append("SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL", self._ed25519.product_serial)
        self.append("SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1", self._p256r1.product_pub)
        self.append("SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE", self._p256r1.product_sig)
        self.append("SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL", self._p256r1.product_serial)

        self.append("SID_PAL_MFG_STORE_MAN_PUB_ED25519", self._ed25519.man_pub)
        self.append("SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE", self._ed25519.man_sig)
        self.append("SID_PAL_MFG_STORE_MAN_ED25519_SERIAL", self._ed25519.man_serial)
        self.append("SID_PAL_MFG_STORE_MAN_PUB_P256R1", self._p256r1.man_pub)
        self.append("SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE", self._p256r1.man_sig)
        self.append("SID_PAL_MFG_STORE_MAN_P256R1_SERIAL", self._p256r1.man_serial)

        self.append("SID_PAL_MFG_STORE_SW_PUB_ED25519", self._ed25519.sw_pub)
        self.append("SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE", self._ed25519.sw_sig)
        self.append("SID_PAL_MFG_STORE_SW_ED25519_SERIAL", self._ed25519.sw_serial)
        self.append("SID_PAL_MFG_STORE_SW_PUB_P256R1", self._p256r1.sw_pub)
        self.append("SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE", self._p256r1.sw_sig)
        self.append("SID_PAL_MFG_STORE_SW_P256R1_SERIAL", self._p256r1.sw_serial)

        self.append("SID_PAL_MFG_STORE_AMZN_PUB_ED25519", self._ed25519.root_pub)
        self.append("SID_PAL_MFG_STORE_AMZN_PUB_P256R1", self._p256r1.root_pub)


class SidMfgOutBin:
    def __init__(self, file_name, config):
        self._file_name = file_name
        self._file = None
        self._config = config
        self._encoded = None

    def __enter__(self):
        path = Path(self._file_name)
        self._file = open(self._file_name, "rb+") if path.is_file() else open(self._file_name, "wb+")
        self._encoded = bytearray(self._file.read())
        _encoded_size = self._config.mfg_page_size * self._config.offset_size
        if len(self._encoded) < _encoded_size:
            self._encoded.extend(bytearray(b"\xff") * (_encoded_size - len(self._encoded)))
        return self

    def __exit__(self, type, value, traceback):
        self._file.seek(0)
        self._file.write(self._encoded)
        self._file.close()

    def write(self, sid_mfg, config):
        encoded_len = len(self._encoded)
        for _ in sid_mfg:
            if encoded_len <= _.end:
                ex_str = "Cannot fit Field-{} in mfg page, mfg_page_size has to be at least {}".format(
                    _.name, int(_.end / self._config.offset_size) + 1
                )
                raise Exception(ex_str)
            self._encoded[_.start : _.end] = _.encoded

            if encoded_len != len(self._encoded):
                raise Exception("Encoded Length Changed")


class SidMfgOutNVM3:
    def __init__(self, file_name):
        self._file_name = file_name
        self._file = None
        self._objs = list()

    def __enter__(self):
        self._file = open(self._file_name, "w+")
        return self

    def __exit__(self, type, value, traceback):
        self._file.write("\n".join(self._objs))
        self._file.close()

    def write(self, sid_mfg):
        if sid_mfg is None:
            raise Exception("mfg is not valid")
        for obj in sid_mfg:
            if not obj.skip:
                self._objs.append(
                    "0x{:04x}:OBJ:{}".format(SidMfgValueId[obj.name].value, binascii.hexlify(obj.encoded).decode())
                )


def main():
    def str2bool(val):
        if isinstance(val, bool):
            return val
        if val.lower() in ("yes", "true", "t", "y", "1"):
            return True
        if val.lower() in ("no", "false", "f", "n", "0"):
            return False
        raise argparse.ArgumentTypeError("Boolean value expected.")

    class _HelpAction(argparse._HelpAction):
        def __call__(self, parser, namespace, values, option_string=None):
            parser.print_help()
            subparsers_actions = [
                action for action in parser._actions if isinstance(action, argparse._SubParsersAction)
            ]
            for subparsers_action in subparsers_actions:
                for choice, subparser in subparsers_action.choices.items():
                    print(subparser.format_help())
            parser.exit()

    common_args = argparse.ArgumentParser(add_help=False)

    common_args.add_argument(
        "--app_srv_pub",
        type=argparse.FileType("rb"),
        required=True,
        help="App server public key",
    )
    common_args.add_argument(
        "--config",
        type=argparse.FileType("r"),
        required=False,
        help="Config Yaml that defines the mfg page offsets",
    )
    common_args.add_argument(
        "--output_bin",
        type=str,
        required=False,
        help="""Output bin file, if this file does not exist
-                             it will be created, if it does exist the data at
-                             the offsets defined in the config file will be
-                             overwritten by provision data""",
    )
    common_args.add_argument(
        "--output_sl_nvm3",
        type=str,
        required=False,
        help="""Create Silabs NVM3 file""",
    )
    common_args.add_argument(
        "--is_network_order",
        type=str2bool,
        required=False,
        default=True,
        nargs="?",
        const=True,
        help="Controls endianess in which integers are stored",
    )

    parser = argparse.ArgumentParser(
        description="""Generate bin file with
                                     sidewalk certificates""",
        add_help=False,
    )
    parser.add_argument(
        "-h", "--help", action=_HelpAction, help="help for help if you need some help"
    )  # add custom help

    subparsers = parser.add_subparsers()

    acs_args = subparsers.add_parser("acs", help="Arguments for ACS Console Input", parents=[common_args])
    acs_args.set_defaults(group="acs")
    acs_args.add_argument(
        "--json",
        type=argparse.FileType("r"),
        required=True,
        help="ACS Console JSON file",
    )

    bb_args = subparsers.add_parser("bb", help="Arguments for Black Box Input", parents=[common_args])
    bb_args.set_defaults(group="bb")
    bb_args.add_argument(
        "--json",
        type=argparse.FileType("r"),
        required=True,
        help="Black Box Sidewalk Response JSON file",
    )

    args = parser.parse_args()
    if args == argparse.Namespace():
        parser.print_help()
        parser.exit()

    app_srv_pub = args.app_srv_pub.read()
    config = Prodict.from_dict(yaml.safe_load(args.config)) if args.config else None

    if args.group == "acs":
        acs_json = json.load(args.json)
        sid_mfg = SidMfgAcsJson(
            acs_json=acs_json,
            app_pub=app_srv_pub,
            config=config,
            is_network_order=args.is_network_order,
        )
    elif args.group == "bb":
        bb_json = json.load(args.json)
        sid_mfg = SidMfgBBJson(
            bb_json=bb_json,
            app_pub=app_srv_pub,
            config=config,
            is_network_order=args.is_network_order,
        )
    else:
        print("Specified group unsupported!")
        sys.exit()

    outfile = None
    if args.output_bin and not args.config:
        print("output_bin and config need to be given at the same time")
        sys.exit()
    elif not args.output_bin and not args.output_sl_nvm3:
        print("At least one output needs to be given")
        sys.exit()
    elif args.output_bin and args.output_sl_nvm3:
        print("Two outputs cannot be given at the same time")
        sys.exit()
    elif args.output_sl_nvm3:
        outfile = args.output_sl_nvm3
        with SidMfgOutNVM3(outfile) as out:
            out.write(sid_mfg)
    elif args.output_bin and args.config:
        outfile = args.output_bin
        with SidMfgOutBin(outfile, config) as out:
            out.write(sid_mfg, config)
    else:
        print("Specified output options unsupported!")
        sys.exit()

    print("WRITE DONE - {}".format(outfile))


if __name__ == "__main__":
    main()

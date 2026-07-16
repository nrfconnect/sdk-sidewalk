#!/usr/bin/env python3
#
# Copyright 2023 Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: MIT-0
#

from __future__ import annotations
import argparse
import binascii
import json
import base64
import sys
import os
import shutil
import subprocess
from ctypes import Structure, c_ubyte
import traceback
import yaml
from intelhex import IntelHex
from enum import Enum
from pathlib import Path
from typing import Union
from typing import List
from typing import Any
from typing import Optional
from typing import Iterator
from dataclasses import dataclass, field

PROVISION_MFG_STORE_VERSION = 7
PROVISION_MFG_STORE_TLV_VERSION = 8

try:
    from rich import print
except ImportError:
    pass

SMSN_SIZE: int = 32
SERIAL_SIZE_WITHOUT_EXPANSION: int = 4
PRK_SIZE: int = 32
ED25519_PUB_SIZE: int = 32
P256R1_PUB_SIZE: int = 64
SIG_SIZE: int = 64

SILABS_MFG_NVM3_KEY_BASE = 0xA9000 # see projects/sid/sal/silabs/sid_pal/include/nvm3_manager.h

# pylint: disable=C0114,C0115,C0116


class AttrDict(dict):
    """
    A class to convert a nested Dictionary into an object with key-values
    that are accessible using attribute notation (AttrDict.attribute) instead of
    key notation (Dict["key"]). This class recursively sets Dicts to objects,
    allowing you to recurse down nested dicts (like: AttrDict.attr.attr)
    """

    # Inspired by:
    # http://stackoverflow.com/a/14620633/1551810
    # http://databio.org/posts/python_AttributeDict.html

    def __init__(self, iterable, **kwargs):
        super(AttrDict, self).__init__(iterable, **kwargs)
        for key, value in iterable.items():
            if isinstance(value, dict):
                self.__dict__[key] = AttrDict(value)
            else:
                self.__dict__[key] = value


def print_subprocess_results(result, subprocess_name="", withAssert=True):
    def check_error_in_line(line):
        return "error" in line.lower()

    for line in result.stdout.decode().splitlines():
        print(line)
        if withAssert:
            assert not check_error_in_line(line), f"Something went wrong after calling subprocess {subprocess_name}"

    for line in result.stderr.decode().splitlines():
        print(line, file=sys.stderr)
        if withAssert:
            assert not check_error_in_line(line), f"Something went wrong after calling subprocess {subprocess_name}"


class SidMfgValueId(Enum):
    """
    Please note that these values have to be in sync at alls times with
    projects/sid/sal/common/public/sid_pal_ifc/mfg_store/sid_pal_mfg_store_ifc.h
    sid_pal_mfg_store_value_t
    """

    """
    Format
    SID_PAL_MFG_STORE_XXXX = (<VALUE>, <SIZE>)
    """

    SID_PAL_MFG_STORE_MAGIC = (0, 4)
    SID_PAL_MFG_STORE_DEVID = (1, 5)
    SID_PAL_MFG_STORE_VERSION = (2, 4)
    SID_PAL_MFG_STORE_SERIAL_NUM = (3, 17)
    SID_PAL_MFG_STORE_SMSN = (4, 32)
    SID_PAL_MFG_STORE_APP_PUB_ED25519 = (5, 32)
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519 = (6, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519 = (7, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE = (8, 64)
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1 = (9, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1 = (10, 64)
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE = (11, 64)
    SID_PAL_MFG_STORE_DAK_PUB_ED25519 = (12, 32)
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE = (13, 64)
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL = (14, None)
    SID_PAL_MFG_STORE_DAK_PUB_P256R1 = (15, 64)
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE = (16, 64)
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL = (17, None)
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519 = (18, 32)
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE = (19, 64)
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL = (20, None)
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1 = (21, 64)
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE = (22, 64)
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL = (23, None)
    SID_PAL_MFG_STORE_MAN_PUB_ED25519 = (24, 32)
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE = (25, 64)
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL = (26, None)
    SID_PAL_MFG_STORE_MAN_PUB_P256R1 = (27, 64)
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE = (28, 64)
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL = (29, None)
    SID_PAL_MFG_STORE_SW_PUB_ED25519 = (30, 32)
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE = (31, 64)
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL = (32, None)
    SID_PAL_MFG_STORE_SW_PUB_P256R1 = (33, 64)
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE = (34, 64)
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL = (35, None)
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519 = (36, 32)
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1 = (37, 64)
    SID_PAL_MFG_STORE_APID = (38, 4)
    SID_PAL_MFG_STORE_CORE_VALUE_MAX = (4000, None)

    def __init__(self, value: int, size: int) -> None:
        # Overload the value so that the enum value corresponds to the
        # Mfg value
        self._value_ = value
        self.size = size


class SidSupportedPlatform(Enum):
    """
    These are the supported sidewalk platforms
    """

    NORDIC = (0, "nordic")
    TI = (1, "ti")
    SILABS = (2, "silabs")
    GENERIC = (3, "generic")

    def __init__(self, value: int, str_name: str) -> None:
        # Overload the value so that the enum value corresponds to the
        # Mfg value
        self._value_ = value
        self.str_name = str_name


@dataclass
class SidArgument:
    name: str
    help: str
    ext: str = ""
    const: str = ""
    handle_class: Any = None
    default: Any = None
    actual_default: Any = None
    required: Any = False
    intype: Any = None
    choices: Any = None
    additional_help: Any = None
    action: str = "store"

    @property
    def arg_name(self) -> str:
        return self.name[2:]


@dataclass
class SidInputGroup:
    name: str
    help: str
    handle_class: Any
    arguments: List[SidArgument]
    common_arguments: List[SidArgument] = field(default_factory=list)


@dataclass
class SidChipAddr:
    name: str
    offset_addr: int
    full_name: str = ""
    mem: int = 0
    default: bool = False

    @property
    def help_str(self) -> str:
        help_str = f"{self.name}"
        if self.full_name:
            help_str += f":{self.full_name}"
        if self.mem:
            help_str += f" mem:{self.mem}"
        if self.offset_addr:
            help_str += f" address: {hex(self.offset_addr)}"
        return help_str


@dataclass
class SidPlatformArgs:
    platform: SidSupportedPlatform
    input_groups: list[SidInputGroup]
    addtional_input_args: list[SidArgument] = field(default_factory=list)
    output_args: list[SidArgument] = field(default_factory=list)
    config_file: Any = None
    chips: list[SidChipAddr] = field(default_factory=list)

    def get_chip_from_name(self, name: str) -> Union[SidChipAddr, None]:
        for _ in self.chips:
            if _.name == name:
                return _
        return None


@dataclass
class SidArgOutContainer:
    platform: SidPlatformArgs
    input: SidInputGroup
    arg: SidArgument
    chip: SidChipAddr


class SidCertMfgCertChain(object):
    @staticmethod
    def get_serial_length(serial):
        sn = int.from_bytes(serial[0:SERIAL_SIZE_WITHOUT_EXPANSION], "little")
        if sn & 0xF0000000 == 0xB0000000:
            # It's a long serial
            return ((sn >> 16) & 0x7F) + 2
        return SERIAL_SIZE_WITHOUT_EXPANSION

    def __init__(self, cert_buffer: bytes, priv: bytes, serial_expansion_supported: bool):
        self._cert_buffer = cert_buffer
        self.device_prk = binascii.unhexlify(priv)
        assert len(self.device_prk) == PRK_SIZE, "Invalid {} private key size -{} Expected Size -{}".format(
            self.chain_name, len(self.device_prk), PRK_SIZE
        )

        def split_bytes(data, length):
            if len(data) < length:
                raise ValueError("Cert chain of {} is shorter than expected".format(self.chain_name))
            return (data[:length], data[length:])

        ca_list = ["device", "dak", "product", "man", "sw", "root"]
        data = cert_buffer
        for ca in ca_list:
            if ca == "device":
                serial_len = SMSN_SIZE
            else:
                serial_len = (
                    self.get_serial_length(data) if serial_expansion_supported else SERIAL_SIZE_WITHOUT_EXPANSION
                )
            (serial, data) = split_bytes(data, serial_len)
            (pubk, data) = split_bytes(data, self.pubk_size)
            (sig, data) = split_bytes(data, SIG_SIZE)
            self.__dict__["{}_serial".format(ca)] = serial
            self.__dict__["{}_pub".format(ca)] = pubk
            self.__dict__["{}_sig".format(ca)] = sig
        self.__dict__["smsn"] = self.__dict__["device_serial"]

        if len(data):
            raise ValueError("Cert chain of {} is longer than expected".format(self.chain_name))

    def __repr__(self) -> str:
        repr_str = f"{self.__class__.__name__}\n"
        for field_name in self.__dict__:  # type: ignore
            repr_str += f" {field_name}: {binascii.hexlify(getattr(self, field_name)).upper()}\n"
        return repr_str


class SidCertMfgP256R1Chain(SidCertMfgCertChain):
    pubk_size = P256R1_PUB_SIZE
    chain_name = "P256R1"

    def __init__(
        self: SidCertMfgP256R1Chain, cert_buffer: bytes, priv: bytes, serial_expansion_supported: bool = True
    ) -> None:
        # self._cert_buffer = cert_buffer
        _device_prk = bytearray(binascii.unhexlify(priv))
        """
        Sometimes cloud generates p256r1 private key with an invalid preceding
        00, handle that case
        """
        if len(_device_prk) == PRK_SIZE + 1 and _device_prk[0] == 00:
            print(f"P256R1 private key size is {PRK_SIZE+1}, truncate to {PRK_SIZE}")
            del _device_prk[0]

        super().__init__(cert_buffer, str(binascii.hexlify(_device_prk), "ascii"), serial_expansion_supported)


class SidCertMfgED25519Chain(SidCertMfgCertChain):
    pubk_size = ED25519_PUB_SIZE
    chain_name = "ED25519"


class SidCertMfgCert:
    @staticmethod
    def from_base64(
        cert: bytes, priv: bytes, is_p256r1: bool, serial_expansion_supported: bool
    ) -> Union[SidCertMfgP256R1Chain, SidCertMfgED25519Chain]:
        if is_p256r1:
            return SidCertMfgP256R1Chain(base64.b64decode(cert), priv, serial_expansion_supported)
        return SidCertMfgED25519Chain(base64.b64decode(cert), priv, serial_expansion_supported)


class SidMfgObj:
    def __init__(
        self: SidMfgObj,
        mfg_enum: SidMfgValueId,
        value: Any,
        info: dict[str, int],
        skip: bool = False,
        word_size: int = 4,
        is_network_order: bool = True,
    ):
        assert isinstance(word_size, int)
        assert (word_size > 0 and info) or (word_size == 0 and not info)

        _info = AttrDict(info) if isinstance(info, dict) else info

        self._name: str = mfg_enum.name
        self._value: Any = value
        self._start: int = 0 if not _info else _info.start
        self._end: int = 0 if not _info else _info.end
        self._word_size: int = word_size
        self._id_val: int = mfg_enum.value
        self._skip: bool = skip

        if info:
            assert self._start < self._end, "Invalid {}  end offset: {} < start offset: {}".format(
                self._name, self._end, self._start
            )
            byte_len = self.end - self.start
        else:
            byte_len = len(value)

        self._encoded: bytes = bytes(bytearray())
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

        if mfg_enum.size is not None and byte_len != mfg_enum.size:
            print(f"{self} has incorrect size {byte_len} expected {mfg_enum.size}")

    @property
    def name(self: SidMfgObj) -> str:
        return self._name

    @property
    def start(self: SidMfgObj) -> int:
        return self._start * self._word_size

    @property
    def end(self: SidMfgObj) -> int:
        return self._end * self._word_size

    @property
    def encoded(self: SidMfgObj) -> bytes:
        return self._encoded

    @property
    def id_val(self: SidMfgObj) -> int:
        return self._id_val

    @property
    def skip(self: SidMfgObj) -> bool:
        return self._skip

    def __repr__(self: SidMfgObj) -> str:
        val: Any = None
        if isinstance(self._value, str):
            val = self._value
        elif isinstance(self._value, int):
            val = self._value
        else:
            val = binascii.hexlify(self._encoded).upper()

        return f"{self._name}[{self.start}:{self.end}] : {val}"


class SidMfg:
    FIXED_POSITION_FIELDS_IN_TLV = [SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, SidMfgValueId.SID_PAL_MFG_STORE_VERSION]

    def __init__(self: SidMfg, app_pub: Union[None, bytes], config: Any, is_network_order: bool) -> None:
        self._config = config
        self._app_pub: Optional[bytes] = app_pub
        self._apid: Optional[str] = None
        self._is_network_order: bool = is_network_order
        self._mfg_objs: List[SidMfgObj] = []
        self._word_size: int = 0 if not self._config else self._config.offset_size
        self._tlv_enabled = self._config.tlv_enabled if self._config and hasattr(self._config, "tlv_enabled") else True

    def __iter__(self: SidMfg) -> Iterator[SidMfgObj]:
        return iter(sorted(self._mfg_objs, key=lambda mfg_obj: mfg_obj.id_val))

    def __repr__(self: SidMfg) -> str:
        # type: ignore
        value = f"{str(self._ed25519)} \n{str(self._p256r1)} \n"
        value += "SID Values\n"
        value += "\n".join([f" {str(_)}" for _ in self._mfg_objs])
        value += "\n"
        return value

    def append(self: SidMfg, mfg_enum: SidMfgValueId, value: Any, can_skip: bool = False) -> None:
        try:
            offset_config = None
            word_size = 0
            if self._config and (not self._tlv_enabled or mfg_enum in self.FIXED_POSITION_FIELDS_IN_TLV):
                offset_config = self._config.mfg_offsets[mfg_enum.name]
                word_size = self._word_size

            mfg_obj = SidMfgObj(
                mfg_enum,
                value,
                offset_config,
                skip=can_skip,
                word_size=word_size,
                is_network_order=self._is_network_order,
            )
            self._mfg_objs.append(mfg_obj)
        except KeyError as ex:
            if can_skip:
                print("Skipping {}".format(mfg_enum.name))
            else:
                raise ex
        except Exception:
            traceback.print_exc()
            exit(1)

    @property
    def mfg_version(self):
        mfg_version = PROVISION_MFG_STORE_TLV_VERSION if self._tlv_enabled else PROVISION_MFG_STORE_VERSION
        return mfg_version.to_bytes(4, byteorder="big" if self._is_network_order else "little")

    @property
    def is_network_order(self):
        return self._is_network_order

    @property
    def word_size(self):
        return self._word_size

    @classmethod
    def from_args(cls, __args__: argparse.Namespace, __pa__) -> None:
        print(f"{cls} is not supported")
        sys.exit(1)


class SidMfgBBJson(SidMfg):
    def __init__(self: SidMfgBBJson, bb_json: Any, config: Any, is_network_order: bool = True) -> None:
        super().__init__(app_pub=None, config=config, is_network_order=is_network_order)

        _bb_json = AttrDict(bb_json)

        def unhex(unhex_val: str) -> bytes:
            return binascii.unhexlify(unhex_val)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self.mfg_version)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVID, unhex(_bb_json.ringNetDevId))
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,
            unhex(_bb_json.PKI.device_cert.ed25519_priv),
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,
            unhex(_bb_json.PKI.device_cert.ed25519_pub),
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
            unhex(_bb_json.PKI.device_cert.ed25519_signature),
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,
            unhex(_bb_json.PKI.device_cert.p256r1_priv),
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,
            unhex(_bb_json.PKI.device_cert.p256r1_pub),
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
            unhex(_bb_json.PKI.device_cert.p256r1_signature),
        )

        for cert in _bb_json.PKI.intermediate_certs:
            _cert = AttrDict(cert)
            if _cert.cert_name == "AMZN":
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519,
                    unhex(_cert.ed25519_pub),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1,
                    unhex(_cert.p256r1_pub),
                )
            elif _cert.cert_name == "MAN":
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519,
                    unhex(_cert.ed25519_pub),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,
                    unhex(_cert.ed25519_signature),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,
                    unhex(_cert.ed25519_serial),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1,
                    unhex(_cert.p256r1_pub),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,
                    unhex(_cert.p256r1_signature),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,
                    unhex(_cert.p256r1_serial),
                )
            elif _cert.cert_name == "MODEL":
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,
                    unhex(_cert.ed25519_pub),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,
                    unhex(_cert.ed25519_signature),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,
                    unhex(_cert.ed25519_serial),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,
                    unhex(_cert.p256r1_pub),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,
                    unhex(_cert.p256r1_signature),
                )
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,
                    unhex(_cert.p256r1_serial),
                )

    @classmethod
    def from_args(cls, args, pa) -> SidMfgBBJson:
        return SidMfgBBJson(
            bb_json=json.load(args.json),
            config=AttrDict(vars(args).get("config", {})),
        )


class SidMfgAcsJson(SidMfg):
    def __init__(
        self: SidMfgAcsJson,
        acs_json: Any,
        app_pub: bytes,
        config: Any,
        is_network_order: bool = True,
    ) -> None:
        super().__init__(app_pub=app_pub, config=config, is_network_order=is_network_order)

        _acs_json = AttrDict(acs_json)
        self._ed25519 = SidCertMfgCert.from_base64(
            _acs_json.eD25519,
            _acs_json.metadata.devicePrivKeyEd25519,
            is_p256r1=False,
            serial_expansion_supported=self._tlv_enabled,
        )
        self._p256r1 = SidCertMfgCert.from_base64(
            _acs_json.p256R1,
            _acs_json.metadata.devicePrivKeyP256R1,
            is_p256r1=True,
            serial_expansion_supported=self._tlv_enabled,
        )
        self._apid = _acs_json.metadata.apid
        self._smsn = binascii.unhexlify(_acs_json.metadata.smsn)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self.mfg_version)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SMSN, self._smsn)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APID, self._apid)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APP_PUB_ED25519, self._app_pub)

        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,
            self._ed25519.device_prk,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, self._ed25519.device_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
            self._ed25519.device_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, self._p256r1.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, self._p256r1.device_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
            self._p256r1.device_sig,
        )

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519, self._ed25519.dak_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,
            self._ed25519.dak_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_ED25519_SERIAL, self._ed25519.dak_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1, self._p256r1.dak_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,
            self._p256r1.dak_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_P256R1_SERIAL, self._p256r1.dak_serial)

        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,
            self._ed25519.product_pub,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,
            self._ed25519.product_sig,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,
            self._ed25519.product_serial,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1, self._p256r1.product_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,
            self._p256r1.product_sig,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,
            self._p256r1.product_serial,
        )

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519, self._ed25519.man_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,
            self._ed25519.man_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL, self._ed25519.man_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1, self._p256r1.man_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,
            self._p256r1.man_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL, self._p256r1.man_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519, self._ed25519.sw_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,
            self._ed25519.sw_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_ED25519_SERIAL, self._ed25519.sw_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1, self._p256r1.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE, self._p256r1.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_P256R1_SERIAL, self._p256r1.sw_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519, self._ed25519.root_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1, self._p256r1.root_pub)

    @classmethod
    def from_args(cls, args, pa) -> SidMfgAcsJson:
        return SidMfgAcsJson(
            acs_json=args.json,
            app_pub=args.app_srv_pub,
            config=AttrDict(vars(args).get("config", {})),
        )


class SidMfgAwsJson(SidMfg):
    def __init__(
        self: SidMfgAwsJson,
        aws_wireless_device_json: Any,
        aws_device_profile_json: Any,
        aws_certificate_json: Any,
        config: Any,
        is_network_order: bool = True,
    ) -> None:
        super().__init__(app_pub=None, config=config, is_network_order=is_network_order)

        _aws_wireless_device_json = AttrDict(aws_wireless_device_json)
        _aws_device_profile_json = AttrDict(aws_device_profile_json)
        _aws_certificate_json = AttrDict(aws_certificate_json)

        def add_generic_sidewalk_filename(json_dict):
            if json_dict and json_dict.get("_SidewalkFileName", None) is None:
                json_dict._SidewalkFileName = "Generic"

        add_generic_sidewalk_filename(_aws_wireless_device_json)
        add_generic_sidewalk_filename(_aws_device_profile_json)
        add_generic_sidewalk_filename(_aws_certificate_json)

        def get_value(crypt_keys: Any, key_type: str) -> Any:
            for _ in crypt_keys:
                _ = AttrDict(_)
                if _.SigningAlg == key_type:
                    return _.Value
            return None

        def unhex(unhex_val: str) -> bytes:
            return binascii.unhexlify(unhex_val)

        if _aws_wireless_device_json and _aws_device_profile_json:
            self._ed25519 = SidCertMfgCert.from_base64(
                get_value(_aws_wireless_device_json.Sidewalk.DeviceCertificates, "Ed25519"),
                get_value(_aws_wireless_device_json.Sidewalk.PrivateKeys, "Ed25519"),
                is_p256r1=False,
                serial_expansion_supported=self._tlv_enabled,
            )
            self._p256r1 = SidCertMfgCert.from_base64(
                get_value(_aws_wireless_device_json.Sidewalk.DeviceCertificates, "P256r1"),
                get_value(_aws_wireless_device_json.Sidewalk.PrivateKeys, "P256r1"),
                is_p256r1=True,
                serial_expansion_supported=self._tlv_enabled,
            )

            _apid = self._get_apid_from_aws_device_profile_json(_aws_device_profile_json)
            if _apid is None:
                print(f"ApId or DeviceTypeId is not found in {_aws_device_profile_json._SidewalkFileName}")
                sys.exit(1)
            else:
                self._apid = _apid

            self._smsn = unhex(_aws_wireless_device_json.Sidewalk.SidewalkManufacturingSn)
            self._app_pub = unhex(_aws_device_profile_json.Sidewalk.ApplicationServerPublicKey)
        elif _aws_certificate_json:
            self._ed25519 = SidCertMfgCert.from_base64(
                _aws_certificate_json.eD25519,
                _aws_certificate_json.metadata.devicePrivKeyEd25519,
                is_p256r1=False,
                serial_expansion_supported=self._tlv_enabled,
            )
            self._p256r1 = SidCertMfgCert.from_base64(
                _aws_certificate_json.p256R1,
                _aws_certificate_json.metadata.devicePrivKeyP256R1,
                is_p256r1=True,
                serial_expansion_supported=self._tlv_enabled,
            )

            self._apid = None
            _apid = _aws_certificate_json.metadata.get("apid", None)
            _deviceTypeId = _aws_certificate_json.metadata.get("deviceTypeId", None)
            if _apid:
                print(f"apid found in {_aws_certificate_json._SidewalkFileName}")
                self._apid = _apid
            if self._apid is None and _deviceTypeId:
                self._apid = _deviceTypeId[-4:]
                print(f"deviceTypeId found in {_aws_certificate_json._SidewalkFileName}")
            if self._apid is None:
                print(f"apid or deviceTypeId not found in {_aws_certificate_json._SidewalkFileName}")
                sys.exit(1)

            self._smsn = unhex(_aws_certificate_json.metadata.smsn)

            _ = _aws_certificate_json.get("applicationServerPublicKey", None)
            if _ is None:
                _ = _aws_certificate_json.get("ApplicationServerPublicKey", None)
            if _ is None:
                print("applicationServerPublicKey not found in certificate_json file")
                sys.exit()

            self._app_pub = unhex(_)
        else:
            print("Error path should not have come here")
            sys.exit()

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self.mfg_version)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SMSN, self._smsn)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APID, self._apid)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APP_PUB_ED25519, self._app_pub)

        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,
            self._ed25519.device_prk,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, self._ed25519.device_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
            self._ed25519.device_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, self._p256r1.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, self._p256r1.device_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
            self._p256r1.device_sig,
        )

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519, self._ed25519.dak_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,
            self._ed25519.dak_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_ED25519_SERIAL, self._ed25519.dak_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1, self._p256r1.dak_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,
            self._p256r1.dak_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_P256R1_SERIAL, self._p256r1.dak_serial)

        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,
            self._ed25519.product_pub,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,
            self._ed25519.product_sig,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,
            self._ed25519.product_serial,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1, self._p256r1.product_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,
            self._p256r1.product_sig,
        )
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,
            self._p256r1.product_serial,
        )

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519, self._ed25519.man_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,
            self._ed25519.man_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL, self._ed25519.man_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1, self._p256r1.man_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,
            self._p256r1.man_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL, self._p256r1.man_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519, self._ed25519.sw_pub)
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,
            self._ed25519.sw_sig,
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_ED25519_SERIAL, self._ed25519.sw_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1, self._p256r1.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE, self._p256r1.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_P256R1_SERIAL, self._p256r1.sw_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519, self._ed25519.root_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1, self._p256r1.root_pub)

    def _get_apid_from_aws_device_profile_json(self, _aws_device_profile_json):
        def _get_device_type_id_from_dak(_aws_device_profile_json):
            search_dak = _aws_device_profile_json.Sidewalk.get("DakCertificateMetadata", [])
            search_dak += _aws_device_profile_json.Sidewalk.get("DAKCertificate", [])
            for _ in search_dak:
                _device_type_id = _.get("DeviceTypeId", None)
                if _device_type_id:
                    return _device_type_id
            return None

        # Find deviceTypeId in dak_certificate
        _device_type_id = _get_device_type_id_from_dak(_aws_device_profile_json)
        if _device_type_id:
            print(f"DeviceTypeId found in {_aws_device_profile_json._SidewalkFileName}")
            # Get last 4 bytes
            return _device_type_id[-4:]

        # If not maybe older certificate
        _apid = _aws_device_profile_json.Sidewalk.get("ApId", None)
        if _apid:
            print(f"ApId found in {_aws_device_profile_json._SidewalkFileName}")
            return _apid
        return None

    @classmethod
    def from_args(cls, args, pa) -> SidMfgAwsJson:
        config = AttrDict(vars(args).get("config", {}))
        if (args.wireless_device_json and not args.device_profile_json) or (
            args.device_profile_json and not args.wireless_device_json
        ):
            pa.error("Provide both --wireless_device_json and --device_profile_json")

        if not (args.wireless_device_json and args.device_profile_json) and not args.certificate_json:
            pa.error("Provide either --wireless_device_json and --device_profile_json or --certificate_json")

        return SidMfgAwsJson(
            aws_wireless_device_json=args.wireless_device_json,
            aws_device_profile_json=args.device_profile_json,
            aws_certificate_json=args.certificate_json,
            config=config,
        )


class SidMfgOutBin:
    def __init__(self: SidMfgOutBin, file_name: str, config: Any) -> None:
        self._file_name = file_name
        self._config = config
        self._tlv_enabled = config.tlv_enabled if config and hasattr(config, "tlv_enabled") else True
        if self._tlv_enabled:
            self._tlv_region_start = (
                config.mfg_offsets[SidMfgValueId.SID_PAL_MFG_STORE_VERSION.name]["end"] * config.offset_size
            )
            self._word_size = config.offset_size
            assert not hasattr(config, "mfg_page_size"), "mfg_page_size is not applicable for TLV mfg page"
        self._encoded = bytearray()
        self._resize_encoded()

    def _resize_encoded(self: SidMfgOutBin):
        if self._tlv_enabled:
            _encoded_size = self._tlv_region_start
        elif self._config:
            _encoded_size = self._config.mfg_page_size * self._config.offset_size
        else:
            _encoded_size = 0

        if len(self._encoded) < _encoded_size:
            self._encoded.extend(bytearray(b"\xff") * (_encoded_size - len(self._encoded)))

    def __enter__(self: SidMfgOutBin) -> SidMfgOutBin:
        path = Path(self._file_name)
        self._file = open(self._file_name, "rb+") if path.is_file() else open(self._file_name, "wb+")
        self._encoded = bytearray(self._file.read())
        self._resize_encoded()
        return self

    def __exit__(self: SidMfgOutBin, type: Any, value: Any, traceback: Any) -> None:
        self._file.seek(0)
        self._file.write(self._encoded)
        self._file.close()

    @property
    def file_name(self):
        return self._file_name

    def remove_value_with_tag(self, tag, is_network_order):
        pos = self._tlv_region_start
        while pos + 4 < len(self._encoded):
            byteorder = "big" if is_network_order else "little"
            record_len = int.from_bytes(self._encoded[pos + 2 : pos + 4], byteorder) + 4
            record_len = (record_len + self._word_size - 1) // self._word_size * self._word_size
            if int.from_bytes(self._encoded[pos : pos + 2], byteorder) == tag:
                del self._encoded[pos : pos + record_len]
                break
            pos += record_len

    def make_tlv(self, tag, encoded_data, is_network_order):
        byteorder = "big" if is_network_order else "little"
        tlv = tag.to_bytes(2, byteorder) + len(encoded_data).to_bytes(2, byteorder) + encoded_data
        # Round to MFG's word_size boundary
        tlv += b"\xff" * ((len(tlv) + self._word_size - 1) // self._word_size * self._word_size - len(tlv))
        return tlv

    def write(self: SidMfgOutBin, sid_mfg: SidMfg) -> None:
        encoded_len = len(self._encoded)
        for _ in sid_mfg:
            if self._tlv_enabled and not _.start and not _.end:
                self.remove_value_with_tag(_.id_val, sid_mfg.is_network_order)
                self._encoded += self.make_tlv(_.id_val, _.encoded, sid_mfg.is_network_order)
                encoded_len = len(self._encoded)
            else:
                if encoded_len < _.end:
                    ex_str = "Cannot fit Field-{} in mfg page, mfg_page_size has to be at least {}".format(
                        _.name, int(_.end / self._config.offset_size) + 1
                    )
                    raise Exception(ex_str)
                self._encoded[_.start : _.end] = _.encoded

            if encoded_len != len(self._encoded):
                raise Exception("Encoded Length Changed")

    def get_output_bin(self: SidMfgOutBin) -> bytes:
        return self._encoded

    @classmethod
    def from_args(cls, __arg_container__: SidArgOutContainer, args, __pa__):
        return cls(
            config=AttrDict(vars(args).get("config", {})),
            file_name=args.output_bin,
        )


class SidMfgOutNVM3:
    def __init__(self: SidMfgOutNVM3, file_name: str, config: Any = {}) -> None:
        self._file_name = file_name
        self._objs: List[str] = list()
        self._config = config

    def __enter__(self: SidMfgOutNVM3) -> SidMfgOutNVM3:
        self._file = open(self._file_name, "w+")
        return self

    def __exit__(self: SidMfgOutNVM3, type: Any, value: Any, traceback: Any) -> None:
        self._file.write("\n".join(self._objs))
        self._file.close()

    @property
    def file_name(self):
        return self._file_name

    def write(self: SidMfgOutNVM3, sid_mfg: SidMfg) -> None:
        if sid_mfg is None:
            raise Exception("mfg is not valid")
        for obj in sid_mfg:
            if not obj.skip:
                new_mfg_value_id = SidMfgValueId[obj.name].value + SILABS_MFG_NVM3_KEY_BASE
                self._objs.append(
                    "0x{:04x}:OBJ:{}".format(
                        new_mfg_value_id,
                        binascii.hexlify(obj.encoded).decode(),
                    )
                )

    def get_output_nvm3(self: SidMfgOutNVM3) -> List[str]:
        return self._objs

    @classmethod
    def from_args(cls, __arg_container__: SidArgOutContainer, args, __pa__):
        return cls(
            config=AttrDict(vars(args).get("config", {})),
            file_name=args.output_nvm3,
        )


class SidMfgOutSLS37:
    def __init__(
        self: SidMfgOutSLS37,
        file_name: str,
        config: Any,
        chip: SidChipAddr,
        commander: Path,
        sl_nvm3: Path,
    ) -> None:
        self._file_name = file_name
        self._objs: List[str] = list()
        self._config = config
        self._chip = chip
        self._commander = commander
        self._sl_nvm3 = sl_nvm3
        self._init_file = f"initfile_{self._chip.name}.s37"

    def __enter__(self: SidMfgOutSLS37) -> SidMfgOutSLS37:
        args = [f"{self._commander}", "--version"]
        result = subprocess.run(args=args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print_subprocess_results(result, " ".join(args))
        return self

    def __exit__(self: SidMfgOutSLS37, __type__: Any, __value__: Any, __traceback__: Any) -> None:
        print("Removing intermediary files")
        try:
            os.remove(self._init_file)
        except OSError:
            pass

    @property
    def file_name(self):
        return self._file_name

    def write(self: SidMfgOutSLS37, __sid_mfg__: SidMfg) -> None:
        print(f"Creating init file for {self._chip.name}")
        gen_init_args = [
            f"{self._commander}",
            "nvm3",
            "initfile",
            "--address",
            f"{self._chip.offset_addr}",
            "--size",
            "0x6000",
            "--device",
            f"{self._chip.full_name}",
            "--outfile",
            f"{self._init_file}",
        ]
        result = subprocess.run(args=gen_init_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print_subprocess_results(result, " ".join(gen_init_args))

        print(f"Creating manufacturing image for {self._chip.name}")
        gen_mfg_args = [
            f"{self._commander}",
            "nvm3",
            "set",
            f"{self._init_file}",
            "--nvm3file",
            f"{self._sl_nvm3}",
            "--outfile",
            f"{self._file_name}",
        ]
        result = subprocess.run(args=gen_mfg_args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print_subprocess_results(result, " ".join(gen_mfg_args))

    @classmethod
    def from_args(cls, arg_container: SidArgOutContainer, args: argparse.Namespace, pa):

        chip = arg_container.platform.get_chip_from_name(args.chip)
        if not chip:
            pa.error(f"Specified chip {args.chip} unsupported!")
            sys.exit(1)

        # Check if nvm3 file has been written
        if not Path(args.output_nvm3).is_file():
            pa.error(f"{args.output_nvm3} has not be written")

        # Overload file name
        if args.output_s37 == arg_container.arg.default(arg_container.platform, arg_container.input, arg_container.arg):
            file_name = f"{arg_container.platform.platform.name.lower()}_{arg_container.input.name}_{chip.name}{'_sv' if args.secure_vault else ''}.{arg_container.arg.ext}"
            args.output_s37 = str(Path.cwd() / Path(file_name))

        return cls(
            config=AttrDict(vars(args).get("config", {})),
            file_name=args.output_s37,
            chip=chip,
            commander=args.commander_bin,
            sl_nvm3=args.output_nvm3,
        )


class SidMfgOutHex:
    def __init__(self: SidMfgOutHex, file_name: str, config: Any, chip: SidChipAddr) -> None:
        self._file_name = file_name
        self._config = config
        self._encoded = None
        self._chip = chip

    def __enter__(self: SidMfgOutHex) -> SidMfgOutHex:
        self._file = open(self._file_name, "w+")
        return self

    def __exit__(self: SidMfgOutHex, __type__: Any, __value__: Any, __traceback__: Any) -> None:
        h = IntelHex()
        h.frombytes(self._encoded, self._chip.offset_addr)
        h.tofile(self._file, "hex")

    @property
    def file_name(self):
        return self._file_name

    def write(self: SidMfgOutHex, sid_mfg: SidMfg) -> None:
        bin = SidMfgOutBin("", self._config)
        bin.write(sid_mfg)
        self._encoded = bin.get_output_bin()

    @classmethod
    def from_args(cls, arg_container: SidArgOutContainer, args: argparse.Namespace, __pa__):
        return cls(
            config=AttrDict(vars(args).get("config", {})),
            file_name=args.output_hex,
            chip=arg_container.chip,
        )


def get_default_config_file(platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument):
    if platform.config_file:
        _ = Path(__file__).parent / platform.config_file
        return str(_)
    return ""


def get_default_output_file(platform: SidPlatformArgs, group: SidInputGroup, argument: SidArgument):
    return Path.cwd() / Path(f"{platform.platform.name.lower()}_{group.name}_CHIP.{argument.ext}")


def is_platform_chip_required(platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument) -> bool:
    return len(platform.chips) != 1


def get_default_platform_chip(platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument) -> str:
    _ = [_ for _ in platform.chips if _.default]
    if _:
        return _[0].name
    return platform.chips[0].name if platform.chips else "None"


def get_additional_addr_help(platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument) -> str:
    test = ""
    for _ in platform.chips:
        test += f"[{_.help_str}]"
    return test


def get_platform_chip_choices(
    platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument
) -> list[str]:
    return sorted(list(set(_.name for _ in platform.chips)))


def get_memory_value_choices(
    platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument
) -> list[int]:
    return sorted(list(set(_.mem for _ in platform.chips)))


def get_default_memory_value(platform: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument) -> int:
    _ = [_ for _ in platform.chips if _.default]
    if _:
        return _[0].mem
    return platform.chips[0].mem


def valid_json_file(val: str) -> dict:
    if val:
        try:
            json_file = open(val, "r")
        except:
            raise argparse.ArgumentTypeError(f"Opening json file {val} failed !")
        try:
            json_data = json.load(json_file)
            json_data["_SidewalkFileName"] = val
            return json_data
        except:
            raise argparse.ArgumentTypeError(f"Invalid json file {val}")
    else:
        return dict({"_SidewalkFileName": val})


def valid_yaml_file(val: str) -> dict:
    if val:
        try:
            yaml_file = open(val, "r")
        except:
            raise argparse.ArgumentTypeError(f"Opening yaml file {val} failed !")
        try:
            return yaml.safe_load(yaml_file)
        except:
            raise argparse.ArgumentTypeError(f"Invalid yaml file {val}")
    return dict({})


def valid_path_to_commander(__platform__: SidPlatformArgs, __group__: SidInputGroup, __argument__: SidArgument) -> str:
    commander_path = shutil.which("commander")
    if commander_path is None and sys.platform == "darwin":
        likely_commander_path = Path("/Applications/Commander.app/Contents/MacOS/commander")
        if os.access(likely_commander_path, os.X_OK):
            return str(likely_commander_path)
    return commander_path if commander_path else ""


def is_file_or_hex(val):
    _ = Path(val)
    if _.is_file():
        with open(_, "rb") as bin_file:
            bin_data = bin_file.read()
    else:
        bin_data = binascii.unhexlify(val)
    if len(bin_data) != 32:
        raise argparse.ArgumentTypeError("32 byte bin data expected.")
    return bin_data


def auto_int(x) -> int:
    return int(x, 0)


CONFIG_FILE_ARG = SidArgument(
    name="--config",
    intype=valid_yaml_file,
    required=False,
    help="Config Yaml that defines the mfg page offsets",
    default=get_default_config_file,
)

OUTPUT_BIN_ARG = SidArgument(
    name="--output_bin",
    ext="bin",
    intype=str,
    required=False,
    default=get_default_output_file,
    handle_class=SidMfgOutBin,
    help="""Output bin file, if this file does not exist
-                             it will be created, if it does exist the data at
-                             the offsets defined in the config file will be
-                             overwritten by provision data""",
)

OUTPUT_HEX_ARG = SidArgument(
    name="--output_hex",
    ext="hex",
    intype=str,
    required=False,
    handle_class=SidMfgOutHex,
    default=get_default_output_file,
    help="""Output hex file, default chip offset is used when generating hexfile""",
)

OUTPUT_SL_NVM3_ARG = SidArgument(
    name="--output_nvm3",
    ext="nvm3",
    intype=str,
    required=False,
    handle_class=SidMfgOutNVM3,
    default=get_default_output_file,
    help="""Create Silabs NVM3 file""",
)

OUTPUT_SL_S37_ARG = SidArgument(
    name="--output_s37",
    ext="s37",
    intype=str,
    required=False,
    handle_class=SidMfgOutSLS37,
    default=get_default_output_file,
    help="""Create Silabs s37 file from commander""",
)

SL_SECURE_VAULT_ARG = SidArgument(
    name="--secure-vault",
    action="store_true",
    help="""Indicates whether secure vault should be used""",
)

DUMP_RAW_VALUES_ARG = SidArgument(
    name="--dump_raw_values",
    action="store_true",
    help="Dump the raw values for debugging",
)

PLATFORM_CHIP_ARG = SidArgument(
    name="--chip",
    intype=str,
    default=get_default_platform_chip,
    choices=get_platform_chip_choices,
    help="Which chip to generate the mfg page",
)

PLATFORM_MEMORY_ARG = SidArgument(
    name="--memory",
    intype=int,
    default=get_default_memory_value,
    choices=get_memory_value_choices,
    help="Memory Footprint",
)

PLATFORM_ADDRESS_ARG = SidArgument(
    name="--addr",
    intype=auto_int,
    help="""Address offset at which mfg page will be stored, this value does not need to be given since
            it is taken from chip argument \n is useful if the default value needs to be overridden""",
    additional_help=get_additional_addr_help,
)

COMMANDER_BIN_ARG = SidArgument(
    name="--commander-bin",
    intype=str,
    default=valid_path_to_commander,
    help="Simplicity Commander tool binary path including binary-name",
)

APP_SRV_PUB_KEY_ARG = SidArgument(
    name="--app_srv_pub",
    intype=is_file_or_hex,
    required=True,
    default=None,
    help="App server public key in bin or hex form",
)

ACS_JSON_ARG = SidArgument(name="--json", intype=valid_json_file, required=True, help="ACS Console JSON file")

BB_JSON_ARG = SidArgument(
    name="--json",
    intype=valid_json_file,
    required=True,
    help="Black Box Sidewalk Response JSON File",
)

AWS_WIRELESS_DEVICE_JSON_ARG = SidArgument(
    name="--wireless_device_json",
    intype=valid_json_file,
    default={},
    required=False,
    help="Json Response of 'aws iotwireless get-wireless-device' ",
)

AWS_DEVICE_PROFILE_JSON_ARG = SidArgument(
    name="--device_profile_json",
    intype=valid_json_file,
    default={},
    required=False,
    help="Json response of 'aws iotwireless get-device-profile ...' ",
)

AWS_CERTIFICATE_JSON_ARG = SidArgument(
    name="--certificate_json",
    intype=valid_json_file,
    default={},
    required=False,
    help="Certificate json generated from sidewalk aws console",
)

COMMON_ARGS = [
    PLATFORM_CHIP_ARG,
    DUMP_RAW_VALUES_ARG,
]

ACS_INPUT_GROUP_FORMAT = SidInputGroup(
    name="acs",
    help="Arguments for ACS Console Input",
    common_arguments=COMMON_ARGS,
    arguments=[
        ACS_JSON_ARG,
        APP_SRV_PUB_KEY_ARG,
    ],
    handle_class=SidMfgAcsJson,
)

BB_INPUT_GROUP_FORMAT = SidInputGroup(
    name="bb",
    help="Arguments for Black Box Input",
    common_arguments=COMMON_ARGS,
    arguments=[
        BB_JSON_ARG,
    ],
    handle_class=SidMfgBBJson,
)

AWS_INPUT_GROUP_FORMAT = SidInputGroup(
    name="aws",
    help="Arguments for AWS Input",
    common_arguments=COMMON_ARGS,
    arguments=[
        AWS_WIRELESS_DEVICE_JSON_ARG,
        AWS_DEVICE_PROFILE_JSON_ARG,
        AWS_CERTIFICATE_JSON_ARG,
    ],
    handle_class=SidMfgAwsJson,
)

ARG_GROUPS = [
    SidPlatformArgs(
        platform=SidSupportedPlatform.NORDIC,
        input_groups=[
            ACS_INPUT_GROUP_FORMAT,
            BB_INPUT_GROUP_FORMAT,
            AWS_INPUT_GROUP_FORMAT,
        ],
        addtional_input_args=[CONFIG_FILE_ARG, PLATFORM_ADDRESS_ARG],
        output_args=[OUTPUT_BIN_ARG, OUTPUT_HEX_ARG],
        config_file=Path("config/nordic/nrf528xx_dk/config.yaml"),
        chips=[SidChipAddr(name="nrf52840", offset_addr=0xFD000, default=True)],
    ),
    SidPlatformArgs(
        platform=SidSupportedPlatform.TI,
        input_groups=[
            ACS_INPUT_GROUP_FORMAT,
            BB_INPUT_GROUP_FORMAT,
            AWS_INPUT_GROUP_FORMAT,
        ],
        addtional_input_args=[CONFIG_FILE_ARG, PLATFORM_ADDRESS_ARG],
        output_args=[
            OUTPUT_BIN_ARG,
            OUTPUT_HEX_ARG,
        ],
        config_file=Path("config/ti/cc13x2_26x2/config.yaml"),
        chips=[
            SidChipAddr(name="P1", full_name="cc1352P1", offset_addr=0x56000),
            SidChipAddr(name="P7", full_name="cc1352P7", offset_addr=0xAE000, default=True),
        ],
    ),
    SidPlatformArgs(
        platform=SidSupportedPlatform.SILABS,
        input_groups=[
            ACS_INPUT_GROUP_FORMAT,
            BB_INPUT_GROUP_FORMAT,
            AWS_INPUT_GROUP_FORMAT,
        ],
        addtional_input_args=[
            COMMANDER_BIN_ARG,
            SL_SECURE_VAULT_ARG,
            PLATFORM_ADDRESS_ARG,
        ],
        output_args=[OUTPUT_SL_NVM3_ARG, OUTPUT_SL_S37_ARG],
        chips=[
            SidChipAddr(name="xg21", full_name="EFR32MG21B020F1024IM32", offset_addr=0x000F8000),
            SidChipAddr(name="xg24", full_name="EFR32MG24BA020F1536GM48", offset_addr=0x08178000, default=True),
            SidChipAddr(name="xg28", full_name="EFR32ZG28B322F1024IM68", offset_addr=0x080F8000),
            SidChipAddr(name="xg23", full_name="EFR32ZG23B020F512IM48", offset_addr=0x8078000),
            SidChipAddr(name="xg25", full_name="EFR32FG25B222F1920IM56", offset_addr=0x081D8000),
        ],
    ),
    SidPlatformArgs(
        platform=SidSupportedPlatform.GENERIC,
        input_groups=[
            ACS_INPUT_GROUP_FORMAT,
            BB_INPUT_GROUP_FORMAT,
            AWS_INPUT_GROUP_FORMAT,
        ],
        addtional_input_args=[CONFIG_FILE_ARG],
        output_args=[OUTPUT_BIN_ARG],
    ),
]


def main() -> None:
    def str2bool(val: Union[bool, str]) -> bool:
        if isinstance(val, bool):
            return val
        if val.lower() in ("yes", "true", "t", "y", "1"):
            return True
        if val.lower() in ("no", "false", "f", "n", "0"):
            return False
        raise argparse.ArgumentTypeError("Boolean value expected.")

    def get_platform_group() -> SidPlatformArgs:
        platform_parser = argparse.ArgumentParser(
            description=f""" Generate mfg page with sidewalk certificates. (Sidewalk MFG Store Version {PROVISION_MFG_STORE_VERSION})""",
            formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        )
        platform_sub_parsers = platform_parser.add_subparsers()

        for _ in [_.platform.str_name for _ in ARG_GROUPS]:
            p_parser = platform_sub_parsers.add_parser(_, help=f"Arguments for {_.capitalize()} Platform")
            p_parser.set_defaults(group=_)

        platform_arg = platform_parser.parse_args(sys.argv[1:2])
        if platform_arg == argparse.Namespace():
            platform_parser.print_help()
            platform_parser.exit()

        return [_ for _ in ARG_GROUPS if _.platform.str_name == platform_arg.group][0]

    platform_group: SidPlatformArgs = get_platform_group()

    parser = argparse.ArgumentParser(
        prog=str(sys.argv[0] + " " + sys.argv[1]),
        description=""" Generate mfg page with sidewalk certificates """,
    )
    subparsers = parser.add_subparsers()
    for input_group in platform_group.input_groups:

        sub = subparsers.add_parser(input_group.name, help=input_group.help)
        sub.set_defaults(group=input_group.name)

        arguments = (
            input_group.arguments
            + input_group.common_arguments
            + platform_group.addtional_input_args
            + platform_group.output_args
        )

        for argument in arguments:

            default = (
                argument.default(platform_group, input_group, argument)
                if callable(argument.default)
                else argument.default
            )
            required = (
                argument.required(platform_group, input_group, argument)
                if callable(argument.required)
                else argument.required
            )
            choices = (
                argument.choices(platform_group, input_group, argument)
                if callable(argument.choices)
                else argument.choices
            )
            help = f"{argument.help} (default: {default})" if default else argument.help
            additional_help = (
                argument.additional_help(platform_group, input_group, argument)
                if callable(argument.additional_help)
                else argument.additional_help
            )
            if additional_help:
                help = f"{help} {additional_help}"

            try:
                if argument.action != "store":
                    sub.add_argument(
                        argument.name,
                        action=argument.action,
                        help=help,
                    )
                else:
                    sub.add_argument(
                        argument.name,
                        type=argument.intype,
                        help=help,
                        required=required,
                        choices=choices,
                        default=default,
                    )
            except Exception as inst:
                print(argument)
                print(inst)
                continue

    args = parser.parse_args(sys.argv[2:])
    if args == argparse.Namespace():
        parser.print_help()
        parser.exit()

    def get_platform_group_from_args_group(args) -> Union[SidInputGroup, None]:
        for _ in platform_group.input_groups:
            if args.group == _.name:
                return _
        return None

    input_group = get_platform_group_from_args_group(args)
    if not input_group:
        parser.error("Specified group unsupported!")

    sid_mfg = input_group.handle_class.from_args(args, pa=parser)

    if args.dump_raw_values:
        print(sid_mfg)

    # Create chip address
    chip_addr = [_ for _ in platform_group.chips if args.chip == _.name ]
    assert len(chip_addr) == 1
    chip_addr = chip_addr[0]
    addr = getattr(args, "addr", None)
    if addr:
        chip_addr.offset_addr = addr
    print(f"Using chip config : ({chip_addr.help_str})")

    for _ in platform_group.output_args:

        arg_container = SidArgOutContainer(platform=platform_group, input=input_group, arg=_, chip=chip_addr)

        # Overload the default name
        file_name = vars(args).get(_.arg_name)
        default_file_name = _.default(platform_group, input_group, _)
        if file_name == default_file_name:
            new_file_name = Path.cwd() / Path(
                f"{platform_group.platform.name.lower()}_{input_group.name}_{chip_addr.name}.{_.ext}"
            )
            vars(args)[_.arg_name] = new_file_name

        with _.handle_class.from_args(arg_container, args, parser) as out:
            out.write(sid_mfg)
            print(f"Generated {out.file_name}")


if __name__ == "__main__":
    main()

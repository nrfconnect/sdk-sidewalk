# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-4-Clause

import verify_license
import unittest


class TestConfiguration(unittest.TestCase):

    def setUp(self):
        self.valid_cfg = {"licenses": [
            {"spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": None,
                "file_regexp": [r".*"]
             }]
        }
        self.full_valid_cfg = self.valid_cfg.copy()
        self.full_valid_cfg["ignore_license_in_files"] = [r".*.txt$"]
        self.full_valid_cfg["supported_file_extensions"] = [r".*.c$"]
        self.full_valid_cfg["license_header_size"] = 123

    def test_import_empty_dict(self):
        empty_dict = dict()
        self.assertRaises(Exception, verify_license.Configuration, empty_dict)

    def test_import_valid_dict(self):
        param_list = [
            self.valid_cfg,
            self.full_valid_cfg
        ]
        for cfg_dict in param_list:
            with self.subTest():
                self.assertIsInstance(verify_license.Configuration(
                    cfg_dict), verify_license.Configuration)

    def __replace_key_in_dict(self, dictionary, key, new_val) -> dict:
        dict_coppy = dictionary.copy()
        dict_coppy[key] = new_val
        return dict_coppy

    def test_import_invalid_dict(self):
        param_list = [
            # valid
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "ignore_license_in_files", [""]), None],
            # not in array
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "ignore_license_in_files", r"*.c"), ValueError],
            # wrong type in array
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "ignore_license_in_files", [123]), ValueError],
            # wrong type
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "ignore_license_in_files", 123), ValueError],

            # valid
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "supported_file_extensions", [""]), None],
            # not in array
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "supported_file_extensions", r"*.c"), ValueError],
            # wrong type in array
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "supported_file_extensions", [123]), ValueError],
            # wrong type
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "supported_file_extensions", 123), ValueError],

            # wrong type in array
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "license_header_size", [123]), ValueError],
            # wrong type
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "license_header_size", "123"), ValueError],
            # valid
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "license_header_size", 123), None],

            # wrong type
            [self.__replace_key_in_dict(
                self.full_valid_cfg, "licenses", 123), ValueError],
            # missing spdx
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "copyright_regexp": "copyright",
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), KeyError],
            # missing copyright
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), KeyError],
            # missing search_license_txt
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "file_regexp": [r".*"]
            }]), KeyError],
            # missing file_regexp
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": None,
            }]), KeyError],
            # copyright in array
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": ["copyright"],
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), ValueError],
            # copyright wrong type
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": 123,
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), ValueError],
            # spdx and license_txt both none
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": None,
                "copyright_regexp": "copyright",
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), ValueError],
            # spdx in array
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": ["example"],
                "copyright_regexp": "copyright",
                "search_license_txt": None,
                "file_regexp": [r".*"]
            }]), ValueError],
            # license text not in array
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": "None",
                "file_regexp": [r".*"]
            }]), ValueError],
            # valid
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": ["None"],
                "file_regexp": [r".*"]
            }]), None],
            # second licnse with error  (license not in array)
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": ["None"],
                "file_regexp": [r".*"]
            }, {
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": "None",
                "file_regexp": [r".*"]
            }]), ValueError],
            # file regexp not in array
            [self.__replace_key_in_dict(self.full_valid_cfg, "licenses", [{
                "spdx": "example",
                "copyright_regexp": "copyright",
                "search_license_txt": ["None"],
                "file_regexp": r".*"
            }]), ValueError],
        ]
        for id, (cfg, exception) in enumerate(param_list):
            with self.subTest(msg=f"id {id}"):
                if exception is not None:
                    self.assertRaises(
                        exception, verify_license.Configuration, cfg)
                else:
                    self.assertIsInstance(verify_license.Configuration(
                        cfg), verify_license.Configuration)


class TestFileListManager(unittest.TestCase):
    def setUp(self):
        valid_cfg_dict = {
            "ignore_license_in_files": [r"file\.txt$", r"ignored_file\.c$"],
            "supported_file_extensions": [r".*\.c$", r".*\.txt"],
            "licenses": [
                {"spdx": "example",
                 "copyright_regexp": "copyright",
                 "search_license_txt": None,
                 "file_regexp": [r".*"]
                 }]
        }
        self.cfg = verify_license.Configuration(valid_cfg_dict)

    def test_file_filtering_on_init(self):
        file_list = ["/path/to/file.txt", "/path/to/CMakeLists.txt",
                     "/not/relative/path.c", "another/path/to/unknown.extension"]
        files_manager = verify_license.FileListManager(self.cfg, file_list)
        self.assertEqual(files_manager.filtered_files, set(
            ["/path/to/CMakeLists.txt", "/not/relative/path.c"]))

    def test_relative_path(self):
        import pathlib
        file_list = ["/path/to/file.txt", "/path/to/CMakeLists.txt",
                     "relative/path.c", "another/path/to/unknown.extension"]
        files_manager = verify_license.FileListManager(self.cfg, file_list)
        self.assertEqual(files_manager.filtered_files, set(
            ["/path/to/CMakeLists.txt", str(pathlib.Path("relative/path.c").absolute())]))

    def test_pathlibPath_as_input(self):
        import pathlib
        file_list = [pathlib.Path("/path/to/file.txt"), pathlib.Path("/path/to/CMakeLists.txt"),
                     pathlib.Path("/not/relative/path.c"), pathlib.Path("another/path/to/unknown.extension")]
        files_manager = verify_license.FileListManager(self.cfg, file_list)
        self.assertEqual(files_manager.filtered_files, set(
            ["/path/to/CMakeLists.txt", "/not/relative/path.c"]))


class TestLicenseVerificator(unittest.TestCase):
    def setUp(self):
        valid_cfg_dict = {
            "ignore_license_in_files": [r"file\.txt$", r"ignored_file\.c$"],
            "supported_file_extensions": [r".*\.c$", r".*\.txt"],
            "licenses": [
                {"spdx": "example",
                 "copyright_regexp": "copyright",
                 "search_license_txt": None,
                 "file_regexp": [r".*"]
                 }]
        }
        self.cfg = verify_license.Configuration(valid_cfg_dict)

    def test_init(self):
        instance = verify_license.LicenseVerificator(
            self.cfg, "dumy/file.path")
        self.assertIsInstance(instance, verify_license.LicenseVerificator)

    def test_find_license_txt(self):
        license_txt = list([
            "very secret file, do not share"
        ])
        file_header = "This license says : \nIt is very secret file, do not share under any circumstance"

        self.assertEqual(0, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_license_txt(
            license_txt, file_header))

    def test_find_license_txt2(self):
        license_txt = list([
            "very secret file, do not share"
        ])
        file_header = "This license says : \nIt is very secret file, sharing is not permited"

        self.assertEqual(1, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_license_txt(
            license_txt, file_header))

    def test_find_spdx(self):
        spdx = "LicenseRef-Nordic-4-Clause"
        file_header = "This license says : \nIt is very secret file, sharing is not permited\n SPDX-License-Identifier: LicenseRef-Nordic-4-Clause"

        self.assertEqual(
            0, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_spdx(spdx, file_header))

    def test_find_spdx2(self):
        spdx = "LicenseRef-Nordic-3-Clause"
        file_header = "This license says : \nIt is very secret file, sharing is not permited\n SPDX-License-Identifier: LicenseRef-Nordic-4-Clause"

        self.assertEqual(
            1, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_spdx(spdx, file_header))

    def test_find_copyright(self):
        copyright_regexp = r"Copyright \d{4}"
        file_header = "Copyright 2099 AwesomeCompany inc.\nThis license says : \nIt is very secret file, sharing is not permited\n SPDX-License-Identifier: LicenseRef-Nordic-4-Clause"

        self.assertEqual(0, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_copyright(
            copyright_regexp, file_header))

    def test_find_copyright2(self):
        copyright_regexp = r"Copyright \d{4}"
        file_header = "Copyright 99 AwesomeCompany inc.\nThis license says : \nIt is very secret file, sharing is not permited\n SPDX-License-Identifier: LicenseRef-Nordic-4-Clause"

        self.assertEqual(1, verify_license.LicenseVerificator(self.cfg, "dumy/file.path").find_copyright(
            copyright_regexp, file_header))

    def test_validate_file_header(self):
        instance = verify_license.LicenseVerificator(
            self.cfg, "dumy/file.path")
        expected_license = {"spdx": "LicenseRef-Nordic-4-Clause",
                            "copyright_regexp": "copyright",
                            "search_license_txt": None,
                            "file_regexp": [r".*"]
                            }
        file_header = "copyright 1099 company.\n\n\nSPDX-License-Identifier: LicenseRef-Nordic-4-Clause\n".split(
            "\n")

        self.assertEqual(0, instance.validate_file_header(
            list(file_header), expected_license))

    def test_validate_file_header2(self):
        instance = verify_license.LicenseVerificator(
            self.cfg, "dumy/file.path")
        expected_license = {"spdx": "example",
                            "copyright_regexp": "copyright",
                            "search_license_txt": None,
                            "file_regexp": [r".*"]
                            }
        file_header = "Lorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.\nIt has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.".split(
            "\n")

        self.assertEqual(2, instance.validate_file_header(
            file_header, expected_license))

    def test_validate_file_header2(self):
        instance = verify_license.LicenseVerificator(
            self.cfg, "dumy/file.path")
        expected_license = {"spdx": "example",
                            "copyright_regexp": "copyright",
                            "search_license_txt": None,
                            "file_regexp": [r".*"]
                            }
        file_header = "copyright \nLorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book.\nIt has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged.\nIt was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.".split(
            "\n")

        self.assertEqual(1, instance.validate_file_header(
            file_header, expected_license))

    def test_validate_file_header2(self):
        instance = verify_license.LicenseVerificator(
            self.cfg, "dumy/file.path")
        expected_license = {"spdx": "LicenseRef-Nordic-4-Clause",
                            "copyright_regexp": "copyright",
                            "search_license_txt": None,
                            "file_regexp": [r".*"]
                            }
        file_header = "SPDX-License-Identifier: LicenseRef-Nordic-4-Clause \n Lorem Ipsum is simply dummy text of the printing and typesetting industry.\nLorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.".split(
            "\n")

        self.assertEqual(1, instance.validate_file_header(
            list(file_header), expected_license))

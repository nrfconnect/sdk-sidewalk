# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import re
import sys
from pathlib import Path

# -- Path setup --------------------------------------------------------------

SIDEWALK_BASE = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path(__file__).resolve().parent / "_extensions"))


def _get_ncs_version() -> tuple[str, str]:
    """Read the pinned NCS version from west.yml for Intersphinx inventories."""
    west_yml = (SIDEWALK_BASE / "west.yml").read_text(encoding="utf-8")
    for block in re.finditer(r"- name: (\w+)\s*\n((?:[ \t]+[^\n]+\n)*)", west_yml):
        if block.group(1) != "nrf":
            continue
        revision = re.search(r"[ \t]+revision:\s*(v[\d.]+)", block.group(2))
        if revision:
            ncs_version = revision.group(1)
            return ncs_version, ncs_version.lstrip("v")
    return "v3.4.1", "3.4.1"


NCS_VERSION, NCS_VERSION_NUMBER = _get_ncs_version()

# -- Project information -----------------------------------------------------

project = "nRF Connect SDK - Amazon Sidewalk"
copyright = "2026, Nordic Semiconductor"
author = "Nordic Semiconductor"
release = "1.3.99"
version = "1.3.99"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.intersphinx",
    "sphinx_tabs.tabs",
    "sphinx_togglebutton",
    "sphinxcontrib.plantuml",
    "sphinx_copybutton",
    "known_issues_filter",
]

templates_path = ["_templates"]

root_doc = "index"

exclude_patterns = ["venv", "_build"]

# -- Options for intersphinx --------------------------------------------------
#
# Hosted NCS inventories are published at nrfconnectdocs.nordicsemi.com.
# The NCS version follows the ``nrf`` project revision in :file:`west.yml`.

_NCS_DOCS_BASE = f"https://nrfconnectdocs.nordicsemi.com/ncs/{NCS_VERSION_NUMBER}"
_NCS_DOCS_STORAGE = f"https://ncsdoc.z6.web.core.windows.net/ncs/{NCS_VERSION_NUMBER}"

intersphinx_mapping = {
    "nrf": (f"{_NCS_DOCS_BASE}/nrf/", f"{_NCS_DOCS_STORAGE}/nrf/objects.inv"),
    "zephyr": (f"{_NCS_DOCS_BASE}/zephyr/", f"{_NCS_DOCS_STORAGE}/zephyr/objects.inv"),
}

# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_ncs_theme"

html_static_path = ["_static"]

html_css_files = ["css/nrf.css", "css/known_issues_filter.css"]

plantuml = "java -jar /usr/local/bin/plantuml.jar"

rst_epilog = """
.. include:: /links.rst
"""

html_theme_options = {
    "docsets": {},
    "ncs_url": "https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/",
    "ncs_label": "nRF Connect SDK Docs",
    "addons_url": "https://nrfconnect.github.io/ncs-app-index/",
    "bare_metal_url": "",
    "logo_url": "https://docs.nordicsemi.com",
}

html_show_sphinx = False

html_extra_path = ["versions.json"]

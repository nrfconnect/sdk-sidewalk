# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'nRF Connect SDK - Amazon Sidewalk'
copyright = '2026, Nordic Semiconductor'
author = 'Nordic Semiconductor'
release = '1.2.99'
version = '1.2.99'

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.intersphinx',
    'sphinx_tabs.tabs',
    'sphinx_togglebutton',
    'sphinxcontrib.plantuml',
    'sphinx_copybutton'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The root document.
root_doc = 'index'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["venv", "_build"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_ncs_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

plantuml = 'java -jar /usr/local/bin/plantuml.jar'

rst_epilog = """
.. include:: /links.rst
"""

# -- Options for sphinx_ncs_theme -------------------------------------------
html_theme_options = {
    'docsets': {},
    "ncs_url": "https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/",
    "ncs_label": "nRF Connect SDK Docs",
    "addons_url": "https://nrfconnect.github.io/ncs-app-index/",
    "bare_metal_url": "",
    "logo_url": "https://docs.nordicsemi.com",
}

html_show_sphinx = False

# Copied into the HTML output for the hosted documentation version switcher.
html_extra_path = ['versions.json']

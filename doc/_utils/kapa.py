#
# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

from sphinx.application import Sphinx


def add_kapa_search(app: Sphinx) -> None:
    """Add Kapa.ai search widget to the documentation build.

    The widget is only added when the ``kapa`` tag is set, i.e. when
    sphinx-build is invoked with ``-t kapa``. This is meant to be enabled at
    CI level for ``latest`` builds only.

    Args:
        app: Sphinx instance.
    """

    if not app.tags.has("kapa"):
        return

    app.add_css_file("css/kapa-widget.css")
    app.add_js_file("js/kapa-widget.js")

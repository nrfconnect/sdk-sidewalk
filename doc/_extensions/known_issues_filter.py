# Copyright (c) 2026 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0

"""
Release filter for the known issues page.
"""

import html
import json
import re
from pathlib import Path
from typing import Any

from docutils import nodes
from docutils.parsers.rst import directives
from sphinx.util.docutils import SphinxDirective

__version__ = "0.1.0"

_VERSION_PATTERN = re.compile(r"^\d+\.\d+\.\d+$")


def _version_class(version: str) -> str:
    return f"v{version.replace('.', '-')}"


class KnownIssuesFilter(SphinxDirective):
    has_content = False
    option_spec = {"default": directives.unchanged}

    def run(self) -> list[nodes.Node]:
        versions_file = Path(self.env.srcdir) / "versions.json"
        versions = json.loads(versions_file.read_text(encoding="utf-8"))
        versions = [
            version for version in versions if _VERSION_PATTERN.fullmatch(version)
        ]

        if not versions:
            raise self.error(
                f'No release versions found in "{versions_file}".')

        default = self.options.get("default", versions[0])
        if default not in versions:
            raise self.error(
                f'Default release "{default}" is not in "{versions_file}".'
            )

        options = ['<option value="all">All releases</option>']
        for version in versions:
            selected = " selected" if version == default else ""
            options.append(
                f'<option value="{_version_class(version)}"{selected}>'
                f"v{html.escape(version)}"
                f"</option>"
            )

        filter_html = f"""
<div class="known-issues-filter">
<label for="known-issues-release">Release:</label>
<select id="known-issues-release" name="known-issues-release">
{"".join(options)}
</select>
</div>
<style>
.known-issues-filter {{
  align-items: center;
  display: flex !important;
  gap: 0.5rem;
  margin: 1rem 0 1.5rem;
  visibility: visible !important;
}}
.known-issues-filter label {{
  font-weight: 700;
}}
.known-issues-filter select,
#known-issues-release {{
  display: inline-block !important;
  visibility: visible !important;
  min-width: 9rem;
  min-height: 2rem;
  padding: 0.35rem 2rem 0.35rem 0.5rem;
  border: 2px solid #999;
  background-color: #fff;
}}
.known-issue-version {{
  border: 1px solid #e97c25;
  color: #e97c25;
  display: inline-block;
  font-size: 0.8em;
  margin-left: 0.5rem;
  padding: 0 0.25rem;
}}
.known-issue-wontfix {{
  border: 1px solid red;
  color: red;
  display: inline-block;
  font-size: 0.8em;
  margin-left: 0.5rem;
  padding: 0 0.25rem;
}}
</style>
<script>
document.addEventListener("DOMContentLoaded", function () {{
  var filter = document.getElementById("known-issues-release");
  if (!filter) {{
    return;
  }}
  var versionPattern = /^v\\d+-\\d+-\\d+$/;
  var root = document.getElementById("known-issues") || document;
  var issues = Array.prototype.filter.call(root.querySelectorAll("dl"), function (issue) {{
    return Array.prototype.some.call(issue.classList, function (name) {{
      return versionPattern.test(name);
    }});
  }});

  issues.forEach(function (issue) {{
    var title = issue.querySelector("dt");
    if (!title) {{
      return;
    }}

    if (issue.classList.contains("wontfix")) {{
      var wontfixTag = document.createElement("span");
      wontfixTag.className = "known-issue-wontfix";
      wontfixTag.textContent = "Won't fix";
      title.appendChild(wontfixTag);
    }}

    Array.prototype.forEach.call(issue.classList, function (name) {{
      if (!versionPattern.test(name)) {{
        return;
      }}
      var versionTag = document.createElement("span");
      versionTag.className = "known-issue-version";
      versionTag.textContent = name.replace(/^v/, "v").split("-").join(".");
      title.appendChild(versionTag);
    }});
  }});

  function applyFilter() {{
    var selected = filter.value;
    issues.forEach(function (issue) {{
      issue.hidden = selected !== "all" && !issue.classList.contains(selected);
    }});
  }}

  filter.addEventListener("change", applyFilter);
  applyFilter();
}});
</script>
"""
        return [nodes.raw("", filter_html, format="html")]


def setup(app) -> dict[str, Any]:
    app.add_directive("known-issues-filter", KnownIssuesFilter)

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }

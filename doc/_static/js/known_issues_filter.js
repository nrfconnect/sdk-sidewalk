// Copyright (c) 2026 Nordic Semiconductor ASA
// SPDX-License-Identifier: Apache-2.0
//
// Release filter for doc/known_issues.rst (same behavior as sdk-edge-ai).

document.addEventListener("DOMContentLoaded", function () {
  var filter = document.getElementById("known-issues-release");
  var root = document.getElementById("known-issues");
  if (!filter || !root) {
    return;
  }

  var versionPattern = /^v\d+-\d+-\d+$/;

  function versionClassNames(element) {
    var names = [];
    for (var i = 0; i < element.classList.length; i++) {
      var name = element.classList[i];
      if (versionPattern.test(name)) {
        names.push(name);
      }
    }
    return names;
  }

  var issues = Array.prototype.filter.call(root.querySelectorAll("dl"), function (issue) {
    return versionClassNames(issue).length > 0;
  });

  issues.forEach(function (issue) {
    var title = issue.querySelector("dt");
    if (!title) {
      return;
    }

    if (issue.classList.contains("wontfix")) {
      var wontfixTag = document.createElement("span");
      wontfixTag.className = "known-issue-wontfix";
      wontfixTag.textContent = "Won't fix";
      title.appendChild(wontfixTag);
    }

    versionClassNames(issue).forEach(function (name) {
      var versionTag = document.createElement("span");
      versionTag.className = "known-issue-version";
      versionTag.textContent = name.replace(/^v/, "v").split("-").join(".");
      title.appendChild(versionTag);
    });
  });

  function issueVisible(issue, selected) {
    if (selected === "all") {
      return true;
    }
    return issue.classList.contains(selected);
  }

  function applyFilter() {
    var selected = filter.value;
    issues.forEach(function (issue) {
      issue.hidden = !issueVisible(issue, selected);
    });
  }

  filter.addEventListener("change", applyFilter);
  applyFilter();
});

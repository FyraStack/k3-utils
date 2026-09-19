#!/usr/bin/env python3
"""Run clang-tidy and convert its diagnostics to SARIF 2.1.0."""

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

DIAGNOSTIC = re.compile(
    r"^(?P<file>.*?):(?P<line>\d+):(?P<column>\d+): "
    r"(?P<severity>warning|error|note): (?P<message>.*?)(?: \[(?P<rule>[^]]+)\])?$"
)


def diagnostic_to_result(match: re.Match[str], root: Path) -> dict[str, object]:
    path = Path(match.group("file"))
    if not path.is_absolute():
        # Meson passes paths such as ../main.cpp; clang-tidy reports those
        # relative paths even though the source tree is the SARIF root.
        while path.parts and path.parts[0] == "..":
            path = Path(*path.parts[1:])
        path = root / path
    relative_path = os.path.relpath(path.resolve(), root.resolve()).replace(os.sep, "/")
    severity = match.group("severity")
    level = (
        "error"
        if severity == "error"
        else "warning"
        if severity == "warning"
        else "note"
    )
    rule = match.group("rule") or "clang-tidy"

    return {
        "ruleId": rule,
        "level": level,
        "message": {"text": match.group("message")},
        "locations": [
            {
                "physicalLocation": {
                    "artifactLocation": {"uri": relative_path},
                    "region": {
                        "startLine": int(match.group("line")),
                        "startColumn": int(match.group("column")),
                    },
                }
            }
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang-tidy", default="clang-tidy")
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--source-root", default=".")
    parser.add_argument("sources", nargs="+")
    args = parser.parse_args()

    root = Path(args.source_root).resolve()
    source_paths = []
    for source in args.sources:
        source_path = Path(source)
        if not source_path.is_absolute():
            while source_path.parts and source_path.parts[0] == "..":
                source_path = Path(*source_path.parts[1:])
            source_path = (root / source_path).resolve()
        source_paths.append(str(source_path))

    command = [
        args.clang_tidy,
        "-p",
        args.build_dir,
        "--config-file",
        str(root / ".clang-tidy"),
        "--warnings-as-errors=*",
        *source_paths,
    ]
    completed = subprocess.run(
        command, cwd=root, capture_output=True, text=True, check=False
    )
    diagnostics = []
    for line in (completed.stdout + completed.stderr).splitlines():
        match = DIAGNOSTIC.match(line)
        if match:
            diagnostics.append(diagnostic_to_result(match, root))
        else:
            print(line, file=sys.stderr)

    rules = {}
    for result in diagnostics:
        rule_id = result["ruleId"]
        rules.setdefault(rule_id, {"id": rule_id, "name": rule_id})

    sarif = {
        "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
        "version": "2.1.0",
        "runs": [
            {
                "tool": {
                    "driver": {
                        "name": "clang-tidy",
                        "informationUri": "https://clang.llvm.org/extra/clang-tidy/",
                        "rules": list(rules.values()),
                    }
                },
                "results": diagnostics,
            }
        ],
    }
    Path(args.output).write_text(json.dumps(sarif, indent=2) + "\n", encoding="utf-8")
    return completed.returncode


if __name__ == "__main__":
    raise SystemExit(main())

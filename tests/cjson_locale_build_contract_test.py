#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
cmake_text = (root / "CMakeLists.txt").read_text(encoding="utf-8")
cjson_text = (root / "third_party/cjson/cJSON.c").read_text(encoding="utf-8")

if "#ifdef ENABLE_LOCALES" not in cjson_text or "localeconv()" not in cjson_text:
    print("FAIL: bundled cJSON no longer exposes its ENABLE_LOCALES locale-aware numeric path")
    sys.exit(2)

pattern = re.compile(
    r"target_compile_definitions\s*\(\s*aeron_cjson\s+PRIVATE(?P<body>.*?)\)",
    re.DOTALL,
)
match = pattern.search(cmake_text)
if not match or not re.search(r"(?:^|\s)ENABLE_LOCALES(?:\s|$)", match.group("body")):
    print("FAIL: aeron_cjson is built without ENABLE_LOCALES")
    sys.exit(1)

print("PASS: aeron_cjson enables cJSON locale-aware numeric parsing")

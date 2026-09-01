from pathlib import Path
import re
import sys

adapter = Path("src/asset/opt_model.c").read_text(encoding="utf-8")
builder = Path("tools/opt2gltf/opt2gltf.c").read_text(encoding="utf-8")
header = Path("tools/opt2gltf/opt2gltf.h").read_text(encoding="utf-8")

checks = [
    (
        "Aeron public overrides are adapted to OptGltf overrides",
        r"OptGltfMaterialOverride\s*\*\s*material_overrides",
        adapter,
    ),
    (
        "Aeron forwards material override count",
        r"\.material_override_count\s*=\s*options->material_override_count",
        adapter,
    ),
    (
        "OptGltf build options carry material overrides",
        r"material_overrides",
        header,
    ),
    (
        "OPT material creation applies the generic override helper",
        r"OptGltf_ApplyMaterialOverrides\s*\(",
        builder,
    ),
]

failed = False
for label, pattern, source in checks:
    if not re.search(pattern, source, re.DOTALL):
        print(f"FAIL: {label}")
        failed = True

if failed:
    sys.exit(1)

print("PASS: generic material overrides propagate through OPT conversion")

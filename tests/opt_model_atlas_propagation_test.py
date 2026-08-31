from pathlib import Path
import re
import sys

source = Path("src/asset/opt_model.c").read_text(encoding="utf-8")

region = re.search(
    r"aeron_gltf_cook_default_options\(&cook_options\);(.*?)aeron_gltf_cook_data",
    source,
    re.DOTALL,
)

if not region:
    print("FAIL: no se encontr? el bloque de configuraci?n del cooker")
    sys.exit(1)

body = region.group(1)

propagation = re.search(
    r"if\s*\(\s*options->max_atlas_size\s*>\s*0\s*\)"
    r"\s*cook_options\.max_atlas_size\s*=\s*options->max_atlas_size\s*;",
    body,
    re.DOTALL,
)

if not propagation:
    print(
        "FAIL: AeronOptModelBuildOptions.max_atlas_size "
        "todav?a no se propaga a AeronGltfCookOptions.max_atlas_size"
    )
    sys.exit(1)

print("PASS: max_atlas_size se propaga al cooker")

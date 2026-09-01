#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "include" / "aeron" / "scene" / "gltf_mesh.h"
LOADER = ROOT / "src" / "scene" / "gltf_mesh.c"
MESH_HEADER = ROOT / "include" / "aeron" / "scene" / "mesh.h"
MESH_SOURCE = ROOT / "src" / "scene" / "mesh.c"

errors = []

def require(text: str, token: str, message: str) -> None:
    if token not in text:
        errors.append(message)

def main() -> None:
    h = HEADER.read_text(encoding="utf-8")
    c = LOADER.read_text(encoding="utf-8")
    mh = MESH_HEADER.read_text(encoding="utf-8")
    mc = MESH_SOURCE.read_text(encoding="utf-8")

    # CPU material state: generic legacy values, no XWAU names.
    for field in (
        "legacy_material",
        "legacy_specular_exponent",
        "legacy_specular_intensity",
        "legacy_specular_color_control",
        "legacy_specular_value",
        "legacy_ambient",
        "normal_scale",
        "legacy_lightness_boost",
        "legacy_saturation_boost",
        "legacy_shadeless",
    ):
        require(h, field, f"AeronGltfMaterial missing CPU legacy field: {field}")

    # Loader must consume the generic extras object and all keys emitted by
    # opt2gltf. Presence is controlled by the numeric metadata flags.
    require(c, "aeronLegacyMaterial", "loader does not parse aeronLegacyMaterial")
    require(c, '"flags"', "loader does not consume legacy metadata flags")
    for key in (
        "specularExponent",
        "specularIntensity",
        "specularColorControl",
        "specularValue",
        "ambient",
        "normalScale",
        "lightnessBoost",
        "saturationBoost",
        "shadeless",
    ):
        require(c, key, f"loader does not consume legacy metadata key: {key}")

    # Legacy numeric values are finite-only, not normalized PBR factors.
    require(c, "isfinite", "loader does not validate legacy numeric finiteness")
    forbidden_loader_clamps = (
        "legacy_specular_exponent < 0.0f",
        "legacy_specular_exponent > 1.0f",
        "legacy_specular_intensity > 1.0f",
        "legacy_specular_color_control > 1.0f",
        "legacy_specular_value > 1.0f",
        "legacy_ambient > 1.0f",
        "normal_scale > 1.0f",
        "legacy_lightness_boost > 1.0f",
        "legacy_saturation_boost > 1.0f",
    )
    for pattern in forbidden_loader_clamps:
        if pattern in c:
            errors.append(f"legacy loader introduces forbidden PBR-style clamp: {pattern}")

    # The GPU ABI appends two float4s after the existing 128 bytes.
    require(mh, "legacy_specular[4]", "GPU material entry missing legacy_specular float4")
    require(mh, "legacy_surface[4]", "GPU material entry missing legacy_surface float4")
    require(mh, "160 B", "GPU material ABI documentation is not updated to 160 B")
    require(mc, "sizeof(AeronPbrMaterialEntry) == 160",
            "GPU material size check is not 160 bytes")

    # Mapping is fixed:
    # legacy_specular = exponent, intensity, color control, value
    expected_spec = (
        "legacy_specular_exponent",
        "legacy_specular_intensity",
        "legacy_specular_color_control",
        "legacy_specular_value",
    )
    expected_surface = (
        "legacy_ambient",
        "normal_scale",
        "legacy_lightness_boost",
        "legacy_saturation_boost",
    )
    for i, field in enumerate(expected_spec):
        if not re.search(
            rf"legacy_specular\[{i}\]\s*=\s*m->{field}\s*;", mc):
            errors.append(
                f"GPU legacy_specular[{i}] is not populated from {field}")
    for i, field in enumerate(expected_surface):
        if not re.search(
            rf"legacy_surface\[{i}\]\s*=\s*m->{field}\s*;", mc):
            errors.append(
                f"GPU legacy_surface[{i}] is not populated from {field}")

    # Existing flags 0x01..0x20 remain intact; new generic legacy mode and
    # shadeless bits are 0x40 and 0x80.
    for bit in ("0x1u", "0x2u", "0x4u", "0x8u", "0x10u", "0x20u"):
        require(mc, bit, f"existing PBR material flag disappeared: {bit}")
    require(mc, "0x40u", "GPU legacy-material-present flag 0x40 is missing")
    require(mc, "0x80u", "GPU legacy-shadeless flag 0x80 is missing")
    if not re.search(r"m->legacy_material[^;\n]*0x40u|0x40u[^;\n]*m->legacy_material", mc):
        errors.append("0x40 is not driven by m->legacy_material")
    if not re.search(r"m->legacy_shadeless[^;\n]*0x80u|0x80u[^;\n]*m->legacy_shadeless", mc):
        errors.append("0x80 is not driven by m->legacy_shadeless")

    # Architecture boundary: no XWAU directive names in generic renderer files.
    for path, text in ((HEADER, h), (LOADER, c), (MESH_HEADER, mh), (MESH_SOURCE, mc)):
        for token in ("Glossiness", "Metallic", "NMIntensity", "SpecularVal",
                      "NoBloom", "AlphaIsntGlass"):
            if re.search(r'"[^"\n]*' + re.escape(token) + r'[^"\n]*"', text):
                errors.append(
                    f"XWAU-specific string leaked into generic Aeron renderer {path.name}: {token}")

    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

    print("PASS: generic legacy material loader and GPU population contract")

if __name__ == "__main__":
    main()

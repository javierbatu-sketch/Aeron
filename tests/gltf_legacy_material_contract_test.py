#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "tools" / "opt2gltf" / "opt2gltf.c"

def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    raise SystemExit(1)

def require(text: str, token: str, message: str) -> None:
    if token not in text:
        fail(message)

def main() -> None:
    text = SOURCE.read_text(encoding="utf-8")

    # The converter, not OpenXWA and not the GPU backend, owns conversion of
    # generic material overrides into the cgltf document.
    require(
        text,
        "OptGltf_ResolveMaterialOverride(",
        "opt2gltf does not resolve generic material overrides per source texture",
    )
    require(
        text,
        "OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE",
        "opt2gltf does not consume the generic normal-image override flag",
    )

    # The normal image must become a real cgltf image/texture bound through the
    # existing glTF normal channel and must have a deterministic URI.
    require(
        text,
        "_Tex%02d_normal.png",
        "normal-image URI is not the deterministic textures/<basename>_TexNN_normal.png form",
    )
    require(
        text,
        "normal_texture",
        "normal-image override is not wired to cgltf_material.normal_texture",
    )

    # Ownership: the caller's RGBA view is temporary. The converter must keep an
    # owned copy in OptGltfDocument::image_pixels, not just borrow the pointer.
    require(
        text,
        "image_pixels",
        "normal image is not stored in OptGltfDocument owned image storage",
    )
    if "normal_image.rgba8" not in text:
        fail("normal-image source view is not consumed")
    if not re.search(
        r"(malloc|calloc)\s*\([^;]{0,220}\bnormal|"
        r"\bnormal[^;]{0,220}(malloc|calloc)\s*\(",
        text,
        flags=re.IGNORECASE | re.DOTALL,
    ):
        fail("no owned allocation is associated with normal-image data")
    if not re.search(
        r"memcpy\s*\([^;]{0,300}normal_image\.rgba8|"
        r"normal_image\.rgba8[^;]{0,300}memcpy\s*\(",
        text,
        flags=re.DOTALL,
    ):
        fail("normal-image RGBA bytes are not copied into owned storage")

    # Legacy scalars must be emitted as generic Aeron material metadata.
    require(text, "aeronLegacyMaterial", "generic legacy material extras object is missing")
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
        require(text, key, f"legacy material metadata key missing: {key}")

    # Existing legacy emissive metadata must survive. Both keys need to be
    # constructed in the same material-metadata region rather than one replacing
    # the other.
    require(text, "aeronEmissiveMode", "existing Aeron emissive metadata was removed")
    legacy_pos = text.find("aeronLegacyMaterial")
    emissive_positions = [
        m.start() for m in re.finditer("aeronEmissiveMode", text)
    ]
    if not emissive_positions or min(abs(legacy_pos - p) for p in emissive_positions) > 7000:
        fail("aeronEmissiveMode and aeronLegacyMaterial are not composed in one material metadata path")

    # XWAU semantics are translated before Aeron. Do not leak XWAU directive
    # names into the generic cgltf metadata contract.
    forbidden_string_literals = (
        "XWAU",
        "Glossiness",
        "Metallic",
        "NoBloom",
        "AlphaIsntGlass",
    )
    for token in forbidden_string_literals:
        if re.search(r'"[^"\n]*' + re.escape(token) + r'[^"\n]*"', text):
            fail(f"XWAU-specific token leaked into Aeron material metadata/string contract: {token}")

    print("PASS: generic legacy glTF material conversion contract")

if __name__ == "__main__":
    main()

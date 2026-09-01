#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]

SHARED = ROOT / "shaders" / "scene_pbr_material_alpha.hlsli"
FORWARD = ROOT / "shaders" / "scene_pbr_mesh_impl.hlsli"
MASK = ROOT / "shaders" / "scene_pbr_prepass_mask.frag.hlsl"

errors = []

def require(text: str, token: str, message: str) -> None:
    if token not in text:
        errors.append(message)

def main() -> None:
    shared = SHARED.read_text(encoding="utf-8")
    forward = FORWARD.read_text(encoding="utf-8")
    mask = MASK.read_text(encoding="utf-8")

    match = re.search(
        r"struct\s+GltfMaterial\s*\{(?P<body>.*?)\};",
        shared,
        re.DOTALL,
    )
    if not match:
        errors.append("shared shader has no GltfMaterial struct")
        body = ""
    else:
        body = match.group("body")

    # CPU AeronPbrMaterialEntry grows by two float4 slots at offsets
    # 128 and 144. StructuredBuffer<T> must use the identical 160-byte
    # element stride even before legacy shading starts consuming them.
    require(
        body,
        "float4 legacy_specular;",
        "GltfMaterial missing legacy_specular float4",
    )
    require(
        body,
        "float4 legacy_surface;",
        "GltfMaterial missing legacy_surface float4",
    )

    # Preserve the existing 128-byte prefix byte-for-byte.
    order = (
        "base_rect",
        "normal_rect",
        "mr_rect",
        "emissive_rect",
        "base_color_factor",
        "emissive_packed",
        "metal_rough",
        "flags",
        "_pad",
        "legacy_specular",
        "legacy_surface",
    )

    positions = []
    for token in order:
        pos = body.find(token)
        if pos < 0:
            positions = []
            break
        positions.append(pos)

    if positions and positions != sorted(positions):
        errors.append(
            "GltfMaterial legacy fields do not append after the existing "
            "128-byte ABI prefix"
        )

    # Default/no-material path must initialize the appended slots.
    require(
        shared,
        "material.legacy_specular = float4(0, 0, 0, 0);",
        "default material does not initialize legacy_specular",
    )
    require(
        shared,
        "material.legacy_surface = float4(0, 0, 0, 0);",
        "default material does not initialize legacy_surface",
    )

    # Current material-buffer readers must share the same definition.
    require(
        forward,
        '#include "scene_pbr_material_alpha.hlsli"',
        "forward PBR path does not use shared GltfMaterial ABI",
    )
    require(
        mask,
        '#include "scene_pbr_material_alpha.hlsli"',
        "alpha-mask prepass does not use shared GltfMaterial ABI",
    )

    # RED5-D is transport/layout only. No XWAU-specific directive names.
    for path, text in ((SHARED, shared), (FORWARD, forward), (MASK, mask)):
        for token in (
            "Glossiness",
            "Metallic",
            "NMIntensity",
            "SpecularVal",
            "NoBloom",
            "AlphaIsntGlass",
        ):
            if re.search(r'"[^"\n]*' + re.escape(token) + r'[^"\n]*"', text):
                errors.append(
                    f"XWAU-specific string leaked into generic shader "
                    f"{path.name}: {token}"
                )

    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

    print("PASS: PBR material shader ABI mirrors the 160-byte CPU layout")

if __name__ == "__main__":
    main()

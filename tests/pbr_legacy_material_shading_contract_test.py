#!/usr/bin/env python3
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SHADER_PATH = ROOT / "shaders" / "scene_pbr_mesh_impl.hlsli"
MATERIAL_PATH = ROOT / "shaders" / "scene_pbr_material_alpha.hlsli"

errors = []

def require(text: str, pattern: str, message: str, flags: int = 0) -> None:
    if not re.search(pattern, text, flags):
        errors.append(message)

def main() -> None:
    shader = SHADER_PATH.read_text(encoding="utf-8")
    material = MATERIAL_PATH.read_text(encoding="utf-8")

    # Legacy data must activate a separate generic shading branch.
    require(
        shader,
        r"m\.flags\s*&\s*GLTF_MATERIAL_LEGACY\b",
        "mesh shader does not branch on GLTF_MATERIAL_LEGACY",
    )
    require(
        shader,
        r"m\.flags\s*&\s*GLTF_MATERIAL_LEGACY_SHADELESS",
        "mesh shader does not branch on GLTF_MATERIAL_LEGACY_SHADELESS",
    )

    # NMIntensity: interpolate from tangent-space neutral (0,0,1).
    require(
        shader,
        r"float3\s+legacy_scale_normal\s*\(",
        "legacy normal-scale helper is missing",
    )
    require(
        shader,
        r"legacy_scale_normal\s*\([^;]*m\.legacy_surface\.y",
        "legacy normal scale is not sourced from legacy_surface.y",
    )
    require(
        shader,
        r"float3\s*\(\s*0(?:\.0f?)?\s*,\s*0(?:\.0f?)?\s*,\s*1(?:\.0f?)?\s*\)",
        "legacy normal scaling does not preserve tangent-space neutral normal",
    )

    # Legacy Metallic is a specular-color/diffuse-character control, not
    # glTF metallic. The reference behavior works in HSV and switches to
    # white specular at the 1.1 threshold.
    require(
        shader,
        r"float3\s+legacy_specular_color\s*\(",
        "legacy specular-color helper is missing",
    )
    require(
        shader,
        r"legacy_rgb_to_hsv\s*\(",
        "legacy specular path does not convert base color to HSV",
    )
    require(
        shader,
        r"legacy_hsv_to_rgb\s*\(",
        "legacy specular path does not convert HSV back to RGB",
    )
    require(
        shader,
        r"metallic\s*<\s*1\.1f",
        "legacy metallic 1.1 threshold is missing",
    )
    require(
        shader,
        r"specular_value\s*\*\s*\(\s*1\.0f\s*-\s*metallic\s*\)",
        "legacy SpecularVal is not applied independently of metallic",
    )
    require(shader, r"saturation_boost", "legacy saturation boost is not used")
    require(shader, r"lightness_boost", "legacy lightness boost is not used")
    require(shader, r"diffuse_scale", "legacy diffuse metallic-character scale is missing")

    # Glossiness/Intensity arrive as generic exponent/intensity. The shader
    # must use a classic pow lobe, not remap exponent to PBR roughness.
    require(
        shader,
        r"float(?:3)?\s+legacy_specular_lobe\s*\(",
        "legacy specular-lobe helper is missing",
    )
    require(
        shader,
        r"pow\s*\(\s*max\s*\(\s*dot\s*\([^)]*\)\s*,\s*0(?:\.0f?)?\s*\)\s*,\s*specular_exponent\s*\)",
        "legacy specular exponent does not drive the pow lobe",
    )
    require(shader, r"specular_intensity", "legacy specular intensity is not applied")
    require(
        shader,
        r"legacy_specular_lobe\s*\([^;]*m\.legacy_specular\.x[^;]*m\.legacy_specular\.y",
        "legacy exponent/intensity are not sourced from legacy_specular.xy",
    )

    # Ambient brightens lit RGB toward base color; it is not AO.
    require(
        shader,
        r"float3\s+legacy_apply_ambient\s*\(",
        "legacy ambient helper is missing",
    )
    require(
        shader,
        r"lit\s*\+\s*ambient\s*\*\s*\(\s*base_color\s*-\s*lit\s*\)",
        "legacy ambient does not brighten toward base color",
    )
    require(
        shader,
        r"legacy_apply_ambient\s*\([^;]*m\.legacy_surface\.x",
        "legacy ambient is not sourced from legacy_surface.x",
    )

    # Shadeless must bypass the regular lighting composition.
    require(
        shader,
        r"legacy_shadeless[\s\S]{0,280}(?:lit|_out\.color)\s*=\s*(?:albedo|float4\s*\(\s*albedo)",
        "legacy shadeless path does not bypass regular lighting",
    )

    # Ordinary non-legacy glTF PBR remains present.
    require(
        shader,
        r"cook_torrance_spec\s*\(",
        "ordinary PBR Cook-Torrance path disappeared",
    )
    require(
        shader,
        r"GLTF_MATERIAL_HAS_METALLIC_ROUGHNESS",
        "ordinary glTF metallic/roughness path disappeared",
    )

    # XWAU directive vocabulary belongs to OpenXWA, not generic Aeron.
    combined = shader + "\n" + material
    for token in ("Glossiness", "NMIntensity", "SpecularVal", "NoBloom",
                  "AlphaIsntGlass"):
        if re.search(r'"[^"\n]*' + re.escape(token) + r'[^"\n]*"', combined):
            errors.append(
                f"XWAU-specific directive leaked into generic Aeron shader: {token}"
            )

    # Explicitly guard the withdrawn PBR shortcut.
    if re.search(
        r"roughness\s*=\s*1(?:\.0f?)?\s*-\s*[^;\n]*legacy_specular",
        shader,
    ):
        errors.append(
            "legacy Glossiness/exponent is incorrectly mapped to PBR roughness"
        )

    if errors:
        for error in errors:
            print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)

    print("PASS: generic legacy material shading contract")

if __name__ == "__main__":
    main()

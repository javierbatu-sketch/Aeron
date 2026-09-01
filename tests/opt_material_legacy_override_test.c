#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "opt_material_override.h"

static int resolve(
    const OptGltfMaterialOverride *items,
    size_t count,
    const char *texture_name,
    size_t texture_index,
    OptGltfMaterialOverride *out,
    bool *has_override,
    opt_error_t *error)
{
    memset(out, 0, sizeof *out);
    *has_override = false;
    memset(error, 0, sizeof *error);
    return OptGltf_ResolveMaterialOverride(
        items, count, texture_name, texture_index,
        out, has_override, error);
}

static int test_default_then_specific_overlay(void)
{
    static const uint8_t pixel[4] = {128u, 128u, 255u, 255u};
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = NULL,
            .flags =
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT |
                OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_SCALE |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS |
                OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE,
            .legacy_specular_exponent = 2.56f,
            .legacy_specular_intensity = 0.125f,
            .legacy_specular_color_control = 0.30f,
            .legacy_specular_value = 0.40f,
            .legacy_ambient = 0.20f,
            .normal_scale = 1.50f,
            .legacy_lightness_boost = 8.0f,
            .legacy_saturation_boost = 1.0f,
            .legacy_shadeless = true,
            .normal_image = { pixel, 1u, 1u },
        },
        {
            .texture_name = "Hull",
            .flags =
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL |
                OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS,
            .legacy_specular_intensity = 2.25f,
            .legacy_specular_color_control = 10.5f,
            .legacy_shadeless = false,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (!resolve(items, 2u, "hUlL", 3u, &out, &has_override, &error)) {
        fprintf(stderr, "FAIL default+specific resolve: %s\n", error.msg);
        return 0;
    }
    if (!has_override) {
        fprintf(stderr, "FAIL resolved override was not reported\n");
        return 0;
    }
    if (out.legacy_specular_exponent != 2.56f ||
        out.legacy_specular_intensity != 2.25f ||
        out.legacy_specular_color_control != 10.5f ||
        out.legacy_specular_value != 0.40f ||
        out.legacy_ambient != 0.20f ||
        out.normal_scale != 1.50f ||
        out.legacy_lightness_boost != 8.0f ||
        out.legacy_saturation_boost != 1.0f ||
        out.legacy_shadeless != false ||
        out.normal_image.rgba8 != pixel ||
        out.normal_image.width != 1u ||
        out.normal_image.height != 1u) {
        fprintf(stderr, "FAIL default/specific fields were not merged exactly\n");
        return 0;
    }
    if ((out.flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT) == 0 ||
        (out.flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY) == 0 ||
        (out.flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL) == 0 ||
        (out.flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS) == 0 ||
        (out.flags & OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE) == 0) {
        fprintf(stderr, "FAIL merged flag presence was lost\n");
        return 0;
    }
    return 1;
}

static int test_default_applies_without_specific(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = NULL,
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT,
            .legacy_ambient = 1.75f,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (!resolve(items, 1u, "AnyTexture", 12u, &out, &has_override, &error) ||
        !has_override ||
        out.legacy_ambient != 1.75f) {
        fprintf(stderr, "FAIL generic default did not apply to arbitrary texture: %s\n",
                error.msg);
        return 0;
    }
    return 1;
}

static int test_generated_alias_is_case_insensitive(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = "tEx07",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE,
            .legacy_specular_value = 3.5f,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (!resolve(items, 1u, "DifferentAuthoredName", 7u,
                 &out, &has_override, &error) ||
        !has_override ||
        out.legacy_specular_value != 3.5f) {
        fprintf(stderr, "FAIL TexNN alias resolution: %s\n", error.msg);
        return 0;
    }
    return 1;
}

static int test_duplicate_defaults_are_error(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = NULL,
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT,
            .legacy_ambient = 0.1f,
        },
        {
            .texture_name = NULL,
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT,
            .legacy_ambient = 0.2f,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (resolve(items, 2u, "Hull", 0u, &out, &has_override, &error) ||
        error.msg[0] == '\0') {
        fprintf(stderr, "FAIL duplicate generic defaults were accepted\n");
        return 0;
    }
    return 1;
}

static int test_duplicate_specific_matches_are_error(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = "Hull",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT,
            .legacy_ambient = 0.1f,
        },
        {
            .texture_name = "Tex02",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE,
            .legacy_specular_value = 0.2f,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (resolve(items, 2u, "Hull", 2u, &out, &has_override, &error) ||
        error.msg[0] == '\0') {
        fprintf(stderr, "FAIL duplicate specific matches were accepted\n");
        return 0;
    }
    return 1;
}

static int test_nonfinite_legacy_is_error(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = NULL,
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT,
            .legacy_specular_exponent = INFINITY,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (resolve(items, 1u, "Hull", 0u, &out, &has_override, &error) ||
        error.msg[0] == '\0') {
        fprintf(stderr, "FAIL non-finite legacy scalar was accepted\n");
        return 0;
    }
    return 1;
}

static int test_existing_pbr_range_rule_is_preserved(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = "Hull",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR,
            .metallic_factor = 1.5f,
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (resolve(items, 1u, "Hull", 0u, &out, &has_override, &error) ||
        error.msg[0] == '\0') {
        fprintf(stderr, "FAIL out-of-range PBR metallic was accepted\n");
        return 0;
    }
    return 1;
}

static int test_invalid_normal_image_is_error(void)
{
    const OptGltfMaterialOverride items[] = {
        {
            .texture_name = NULL,
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE,
            .normal_image = { NULL, 1u, 1u },
        },
    };
    OptGltfMaterialOverride out;
    bool has_override;
    opt_error_t error = {{0}};

    if (resolve(items, 1u, "Hull", 0u, &out, &has_override, &error) ||
        error.msg[0] == '\0') {
        fprintf(stderr, "FAIL invalid normal image was accepted\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    if (!test_default_then_specific_overlay()) return 1;
    if (!test_default_applies_without_specific()) return 1;
    if (!test_generated_alias_is_case_insensitive()) return 1;
    if (!test_duplicate_defaults_are_error()) return 1;
    if (!test_duplicate_specific_matches_are_error()) return 1;
    if (!test_nonfinite_legacy_is_error()) return 1;
    if (!test_existing_pbr_range_rule_is_preserved()) return 1;
    if (!test_invalid_normal_image_is_error()) return 1;

    puts("PASS: generic legacy material default/specific resolution");
    return 0;
}

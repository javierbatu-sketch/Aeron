#include <stdio.h>
#include <string.h>

#include "opt_material_override.h"

static int test_real_texture_name_and_partial_override(void)
{
    const OptGltfMaterialOverride overrides[] = {
        {
            .texture_name = "Hull",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR,
            .metallic_factor = 0.25f,
            .roughness_factor = 0.10f,
        },
    };
    cgltf_material material;
    opt_error_t error = {{0}};

    memset(&material, 0, sizeof material);
    material.has_pbr_metallic_roughness = 1;
    material.pbr_metallic_roughness.metallic_factor = 0.0f;
    material.pbr_metallic_roughness.roughness_factor = 1.0f;

    if (!OptGltf_ApplyMaterialOverrides(
            overrides, 1, "hUlL", 0, &material, &error)) {
        fprintf(stderr, "FAIL real-name apply: %s\n", error.msg);
        return 0;
    }

    if (material.pbr_metallic_roughness.metallic_factor != 0.25f) {
        fprintf(stderr, "FAIL metallic factor was not overridden\n");
        return 0;
    }
    if (material.pbr_metallic_roughness.roughness_factor != 1.0f) {
        fprintf(stderr, "FAIL absent roughness override changed default\n");
        return 0;
    }

    return 1;
}

static int test_generated_tex_alias(void)
{
    const OptGltfMaterialOverride overrides[] = {
        {
            .texture_name = "Tex01",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR,
            .metallic_factor = 0.90f,
            .roughness_factor = 0.35f,
        },
    };
    cgltf_material material;
    opt_error_t error = {{0}};

    memset(&material, 0, sizeof material);
    material.has_pbr_metallic_roughness = 1;
    material.pbr_metallic_roughness.metallic_factor = 0.0f;
    material.pbr_metallic_roughness.roughness_factor = 1.0f;

    if (!OptGltf_ApplyMaterialOverrides(
            overrides, 1, "AuthoredTextureName", 1, &material, &error)) {
        fprintf(stderr, "FAIL TexNN alias apply: %s\n", error.msg);
        return 0;
    }

    if (material.pbr_metallic_roughness.metallic_factor != 0.0f) {
        fprintf(stderr, "FAIL absent metallic override changed default\n");
        return 0;
    }
    if (material.pbr_metallic_roughness.roughness_factor != 0.35f) {
        fprintf(stderr, "FAIL roughness factor was not overridden\n");
        return 0;
    }

    return 1;
}

static int test_duplicate_resolution_is_error(void)
{
    const OptGltfMaterialOverride overrides[] = {
        {
            .texture_name = "Hull",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR,
            .metallic_factor = 0.20f,
        },
        {
            .texture_name = "Tex02",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR,
            .roughness_factor = 0.60f,
        },
    };
    cgltf_material material;
    opt_error_t error = {{0}};

    memset(&material, 0, sizeof material);
    material.has_pbr_metallic_roughness = 1;
    material.pbr_metallic_roughness.roughness_factor = 1.0f;

    if (OptGltf_ApplyMaterialOverrides(
            overrides, 2, "Hull", 2, &material, &error)) {
        fprintf(stderr, "FAIL duplicate material overrides were accepted\n");
        return 0;
    }
    if (error.msg[0] == '\0') {
        fprintf(stderr, "FAIL duplicate material overrides gave no error\n");
        return 0;
    }

    return 1;
}

static int test_invalid_generic_factor_is_error(void)
{
    const OptGltfMaterialOverride overrides[] = {
        {
            .texture_name = "Hull",
            .flags = OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR,
            .metallic_factor = 1.50f,
        },
    };
    cgltf_material material;
    opt_error_t error = {{0}};

    memset(&material, 0, sizeof material);
    material.has_pbr_metallic_roughness = 1;

    if (OptGltf_ApplyMaterialOverrides(
            overrides, 1, "Hull", 0, &material, &error)) {
        fprintf(stderr, "FAIL invalid generic PBR factor was accepted\n");
        return 0;
    }

    return 1;
}

int main(void)
{
    if (!test_real_texture_name_and_partial_override())
        return 1;
    if (!test_generated_tex_alias())
        return 1;
    if (!test_duplicate_resolution_is_error())
        return 1;
    if (!test_invalid_generic_factor_is_error())
        return 1;

    puts("PASS: generic OPT material override matching/application");
    return 0;
}

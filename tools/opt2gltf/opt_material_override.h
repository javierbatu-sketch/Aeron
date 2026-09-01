#ifndef AERON_OPT_MATERIAL_OVERRIDE_H
#define AERON_OPT_MATERIAL_OVERRIDE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cgltf.h"
#include "opt.h"

enum {
    OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR = 1u << 0,
    OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR = 1u << 1,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT = 1u << 2,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY = 1u << 3,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL = 1u << 4,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE = 1u << 5,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT = 1u << 6,
    OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_SCALE = 1u << 7,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST = 1u << 8,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST = 1u << 9,
    OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS = 1u << 10,
    OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE = 1u << 11,
};

typedef struct OptGltfMaterialImage {
    const uint8_t *rgba8;
    uint32_t width;
    uint32_t height;
} OptGltfMaterialImage;

typedef struct OptGltfMaterialOverride {
    const char *texture_name;
    uint32_t flags;
    float metallic_factor;
    float roughness_factor;
    float legacy_specular_exponent;
    float legacy_specular_intensity;
    float legacy_specular_color_control;
    float legacy_specular_value;
    float legacy_ambient;
    float normal_scale;
    float legacy_lightness_boost;
    float legacy_saturation_boost;
    bool legacy_shadeless;
    OptGltfMaterialImage normal_image;
} OptGltfMaterialOverride;

bool OptGltf_ResolveMaterialOverride(
    const OptGltfMaterialOverride *overrides,
    size_t override_count,
    const char *texture_name,
    int32_t texture_index,
    OptGltfMaterialOverride *out_override,
    bool *out_has_override,
    opt_error_t *error);

bool OptGltf_ApplyMaterialOverrides(
    const OptGltfMaterialOverride *overrides,
    size_t override_count,
    const char *texture_name,
    int32_t texture_index,
    cgltf_material *material,
    opt_error_t *error);

#endif

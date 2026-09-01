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
};

typedef struct OptGltfMaterialOverride {
    const char *texture_name;
    uint32_t flags;
    float metallic_factor;
    float roughness_factor;
} OptGltfMaterialOverride;

bool OptGltf_ApplyMaterialOverrides(
    const OptGltfMaterialOverride *overrides,
    size_t override_count,
    const char *texture_name,
    int32_t texture_index,
    cgltf_material *material,
    opt_error_t *error);

#endif

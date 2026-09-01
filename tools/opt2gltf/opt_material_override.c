#include "opt_material_override.h"

#include <math.h>
#include <stdio.h>

static int material_name_equal(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a++;
        unsigned char cb = (unsigned char)*b++;
        if (ca >= 'a' && ca <= 'z') ca = (unsigned char)(ca - ('a' - 'A'));
        if (cb >= 'a' && cb <= 'z') cb = (unsigned char)(cb - ('a' - 'A'));
        if (ca != cb) return 0;
    }
    return *a == *b;
}

static bool material_override_valid(
    const OptGltfMaterialOverride *item, size_t index, opt_error_t *error)
{
    const uint32_t known_flags =
        OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR |
        OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR;

    if (!item->texture_name || !item->texture_name[0] ||
        (item->flags & ~known_flags) != 0 ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR) &&
         (!isfinite(item->metallic_factor) ||
          item->metallic_factor < 0.0f || item->metallic_factor > 1.0f)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR) &&
         (!isfinite(item->roughness_factor) ||
          item->roughness_factor < 0.0f || item->roughness_factor > 1.0f))) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "invalid material override %zu", index);
        return false;
    }
    return true;
}

bool OptGltf_ApplyMaterialOverrides(
    const OptGltfMaterialOverride *overrides,
    size_t override_count,
    const char *texture_name,
    int32_t texture_index,
    cgltf_material *material,
    opt_error_t *error)
{
    if (error) error->msg[0] = '\0';
    if (!material || (override_count && !overrides)) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "invalid material override arguments");
        return false;
    }

    char generated_name[32];
    snprintf(generated_name, sizeof generated_name, "Tex%02d",
             (int)texture_index);

    const OptGltfMaterialOverride *match = NULL;
    size_t match_count = 0;
    for (size_t i = 0; i < override_count; ++i) {
        const OptGltfMaterialOverride *item = &overrides[i];
        if (!material_override_valid(item, i, error))
            return false;
        if (material_name_equal(item->texture_name, texture_name) ||
            material_name_equal(item->texture_name, generated_name)) {
            match = item;
            ++match_count;
        }
    }

    if (match_count > 1) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "multiple material overrides resolve to texture '%s'",
                     texture_name ? texture_name : generated_name);
        return false;
    }
    if (!match)
        return true;
    if (!material->has_pbr_metallic_roughness) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "material override target has no PBR material");
        return false;
    }

    if (match->flags & OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR)
        material->pbr_metallic_roughness.metallic_factor =
            match->metallic_factor;
    if (match->flags & OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR)
        material->pbr_metallic_roughness.roughness_factor =
            match->roughness_factor;
    return true;
}

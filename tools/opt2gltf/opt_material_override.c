#include "opt_material_override.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

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

static uint32_t material_override_known_flags(void)
{
    return
        OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR |
        OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT |
        OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_SCALE |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST |
        OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS |
        OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE;
}

static bool material_override_valid(
    const OptGltfMaterialOverride *item, size_t index, opt_error_t *error)
{
    const uint32_t known_flags = material_override_known_flags();

    if ((item->texture_name && !item->texture_name[0]) ||
        (item->flags & ~known_flags) != 0 ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR) &&
         (!isfinite(item->metallic_factor) ||
          item->metallic_factor < 0.0f || item->metallic_factor > 1.0f)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR) &&
         (!isfinite(item->roughness_factor) ||
          item->roughness_factor < 0.0f || item->roughness_factor > 1.0f)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT) &&
         !isfinite(item->legacy_specular_exponent)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY) &&
         !isfinite(item->legacy_specular_intensity)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL) &&
         !isfinite(item->legacy_specular_color_control)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE) &&
         !isfinite(item->legacy_specular_value)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT) &&
         !isfinite(item->legacy_ambient)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_SCALE) &&
         !isfinite(item->normal_scale)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST) &&
         !isfinite(item->legacy_lightness_boost)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST) &&
         !isfinite(item->legacy_saturation_boost)) ||
        ((item->flags & OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE) &&
         (!item->normal_image.rgba8 ||
          !item->normal_image.width ||
          !item->normal_image.height))) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "invalid material override %zu", index);
        return false;
    }
    return true;
}

static void material_override_overlay(
    OptGltfMaterialOverride *out,
    const OptGltfMaterialOverride *item)
{
    if (item->texture_name)
        out->texture_name = item->texture_name;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR)
        out->metallic_factor = item->metallic_factor;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR)
        out->roughness_factor = item->roughness_factor;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT)
        out->legacy_specular_exponent = item->legacy_specular_exponent;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY)
        out->legacy_specular_intensity = item->legacy_specular_intensity;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL)
        out->legacy_specular_color_control = item->legacy_specular_color_control;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE)
        out->legacy_specular_value = item->legacy_specular_value;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_AMBIENT)
        out->legacy_ambient = item->legacy_ambient;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_SCALE)
        out->normal_scale = item->normal_scale;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST)
        out->legacy_lightness_boost = item->legacy_lightness_boost;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST)
        out->legacy_saturation_boost = item->legacy_saturation_boost;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_LEGACY_SHADELESS)
        out->legacy_shadeless = item->legacy_shadeless;
    if (item->flags & OPT_GLTF_MATERIAL_OVERRIDE_NORMAL_IMAGE)
        out->normal_image = item->normal_image;
    out->flags |= item->flags;
}

bool OptGltf_ResolveMaterialOverride(
    const OptGltfMaterialOverride *overrides,
    size_t override_count,
    const char *texture_name,
    int32_t texture_index,
    OptGltfMaterialOverride *out_override,
    bool *out_has_override,
    opt_error_t *error)
{
    if (error) error->msg[0] = '\0';
    if (!out_override || !out_has_override ||
        (override_count && !overrides)) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "invalid material override arguments");
        return false;
    }

    memset(out_override, 0, sizeof *out_override);
    *out_has_override = false;

    char generated_name[32];
    snprintf(generated_name, sizeof generated_name, "Tex%02d",
             (int)texture_index);

    const OptGltfMaterialOverride *default_match = NULL;
    const OptGltfMaterialOverride *specific_match = NULL;
    size_t default_count = 0;
    size_t specific_count = 0;

    for (size_t i = 0; i < override_count; ++i) {
        const OptGltfMaterialOverride *item = &overrides[i];
        if (!material_override_valid(item, i, error))
            return false;

        if (!item->texture_name) {
            default_match = item;
            ++default_count;
        } else if (material_name_equal(item->texture_name, texture_name) ||
                   material_name_equal(item->texture_name, generated_name)) {
            specific_match = item;
            ++specific_count;
        }
    }

    if (default_count > 1) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "multiple default material overrides");
        return false;
    }
    if (specific_count > 1) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "multiple material overrides resolve to texture '%s'",
                     texture_name ? texture_name : generated_name);
        return false;
    }

    if (default_match)
        material_override_overlay(out_override, default_match);
    if (specific_match)
        material_override_overlay(out_override, specific_match);

    *out_has_override = default_match != NULL || specific_match != NULL;
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
    if (!material) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "invalid material override arguments");
        return false;
    }

    OptGltfMaterialOverride resolved;
    bool has_override = false;
    if (!OptGltf_ResolveMaterialOverride(
            overrides, override_count, texture_name, texture_index,
            &resolved, &has_override, error))
        return false;

    if (!has_override)
        return true;

    const uint32_t pbr_flags =
        OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR |
        OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR;

    if ((resolved.flags & pbr_flags) &&
        !material->has_pbr_metallic_roughness) {
        if (error)
            snprintf(error->msg, sizeof error->msg,
                     "material override target has no PBR material");
        return false;
    }

    if (resolved.flags & OPT_GLTF_MATERIAL_OVERRIDE_METALLIC_FACTOR)
        material->pbr_metallic_roughness.metallic_factor =
            resolved.metallic_factor;
    if (resolved.flags & OPT_GLTF_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR)
        material->pbr_metallic_roughness.roughness_factor =
            resolved.roughness_factor;

    return true;
}

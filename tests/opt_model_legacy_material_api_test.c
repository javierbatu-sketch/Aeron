#include <stdbool.h>
#include <stdint.h>

#include "aeron/asset/opt_model.h"

int main(void)
{
    static const uint8_t pixel[4] = {128u, 128u, 255u, 255u};

    AeronOptMaterialOverride override = {0};
    AeronOptModelBuildOptions options = {0};

    override.texture_name = NULL;
    override.flags =
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_AMBIENT |
        AERON_OPT_MATERIAL_OVERRIDE_NORMAL_SCALE |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST |
        AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SHADELESS |
        AERON_OPT_MATERIAL_OVERRIDE_NORMAL_IMAGE;

    override.legacy_specular_exponent = 2.56f;
    override.legacy_specular_intensity = 0.125f;
    override.legacy_specular_color_control = 1.25f;
    override.legacy_specular_value = 0.4f;
    override.legacy_ambient = 0.2f;
    override.normal_scale = 1.5f;
    override.legacy_lightness_boost = 8.0f;
    override.legacy_saturation_boost = 1.0f;
    override.legacy_shadeless = true;
    override.normal_image.rgba8 = pixel;
    override.normal_image.width = 1u;
    override.normal_image.height = 1u;

    options.material_overrides = &override;
    options.material_override_count = 1u;

    if (options.material_overrides != &override ||
        options.material_override_count != 1u ||
        override.texture_name != NULL ||
        override.legacy_specular_exponent != 2.56f ||
        override.legacy_specular_intensity != 0.125f ||
        override.legacy_specular_color_control != 1.25f ||
        override.legacy_specular_value != 0.4f ||
        override.legacy_ambient != 0.2f ||
        override.normal_scale != 1.5f ||
        override.legacy_lightness_boost != 8.0f ||
        override.legacy_saturation_boost != 1.0f ||
        !override.legacy_shadeless ||
        override.normal_image.rgba8 != pixel ||
        override.normal_image.width != 1u ||
        override.normal_image.height != 1u) {
        return 1;
    }

    return 0;
}

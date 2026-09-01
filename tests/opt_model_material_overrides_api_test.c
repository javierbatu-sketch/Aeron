#include <stdint.h>

#include "aeron/asset/opt_model.h"

int main(void)
{
    AeronOptMaterialOverride override = {0};
    AeronOptModelBuildOptions options = {0};

    override.texture_name = "Hull";
    override.flags =
        AERON_OPT_MATERIAL_OVERRIDE_METALLIC_FACTOR |
        AERON_OPT_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR;
    override.metallic_factor = 0.25f;
    override.roughness_factor = 0.75f;

    options.material_overrides = &override;
    options.material_override_count = 1;

    if (options.material_overrides != &override ||
        options.material_override_count != 1 ||
        override.metallic_factor != 0.25f ||
        override.roughness_factor != 0.75f) {
        return 1;
    }

    return 0;
}

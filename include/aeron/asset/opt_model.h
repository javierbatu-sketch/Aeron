#ifndef AERON_ASSET_OPT_MODEL_H
#define AERON_ASSET_OPT_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "aeron/asset/flight_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Native LucasArts OPT coordinates use 65536 units for 1600 metres. */
#define AERON_OPT_METERS_PER_UNIT (1600.0f / 65536.0f)
#define AERON_OPT_UNITS_PER_METER (65536.0f / 1600.0f)

enum {
	AERON_OPT_MATERIAL_OVERRIDE_METALLIC_FACTOR = 1u << 0,
	AERON_OPT_MATERIAL_OVERRIDE_ROUGHNESS_FACTOR = 1u << 1,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_EXPONENT = 1u << 2,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_INTENSITY = 1u << 3,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_COLOR_CONTROL = 1u << 4,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SPECULAR_VALUE = 1u << 5,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_AMBIENT = 1u << 6,
	AERON_OPT_MATERIAL_OVERRIDE_NORMAL_SCALE = 1u << 7,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_LIGHTNESS_BOOST = 1u << 8,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SATURATION_BOOST = 1u << 9,
	AERON_OPT_MATERIAL_OVERRIDE_LEGACY_SHADELESS = 1u << 10,
	AERON_OPT_MATERIAL_OVERRIDE_NORMAL_IMAGE = 1u << 11,
};

typedef struct AeronOptMaterialImage {
	const uint8_t* rgba8;
	uint32_t width;
	uint32_t height;
} AeronOptMaterialImage;

typedef struct AeronOptMaterialOverride {
	const char* texture_name;
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
	AeronOptMaterialImage normal_image;
} AeronOptMaterialOverride;

typedef struct AeronOptModelBuildOptions {
	float smooth_angle_degrees;
	float emissive_strength;
	bool emissive;
	const struct AeronOptAlphaOverride* alpha_overrides;
	size_t alpha_override_count;
	const AeronOptMaterialOverride* material_overrides;
	size_t material_override_count;
	/* 0 keeps the glTF cooker's current default. */
	int max_atlas_size;
} AeronOptModelBuildOptions;

typedef struct AeronOptAlphaOverride {
	const char* texture_name;
	AeronGltfAlphaMode alpha_mode;
	float alpha_cutoff;
} AeronOptAlphaOverride;

typedef struct AeronOptModelError {
	int code;
	char message[256];
} AeronOptModelError;

bool Aeron_OptModelBuildMemory(
		const void *bytes,
		size_t size,
		const char *label,
		const AeronOptModelBuildOptions *options,
		AeronFlightModel *out,
		AeronOptModelError *error);

#ifdef __cplusplus
}
#endif

#endif

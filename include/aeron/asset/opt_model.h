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
};

typedef struct AeronOptMaterialOverride {
	const char* texture_name;
	uint32_t flags;
	float metallic_factor;
	float roughness_factor;
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

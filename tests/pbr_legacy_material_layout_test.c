#include <stddef.h>
#include "aeron/scene/mesh.h"

_Static_assert(sizeof(AeronPbrMaterialEntry) == 160,
               "legacy PBR material storage must be 160 bytes");
_Static_assert(offsetof(AeronPbrMaterialEntry, legacy_specular) == 128,
               "legacy_specular must append after the existing 128-byte ABI");
_Static_assert(offsetof(AeronPbrMaterialEntry, legacy_surface) == 144,
               "legacy_surface must occupy the final 16-byte slot");

int main(void)
{
    return 0;
}

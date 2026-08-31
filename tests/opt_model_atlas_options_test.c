#include "aeron/asset/opt_model.h"

int main(void)
{
    AeronOptModelBuildOptions options = {0};
    options.max_atlas_size = 8192;

    return options.max_atlas_size == 8192 ? 0 : 1;
}

#ifndef AERON_INTERNAL_PRIMITIVE_COMPACT_H
#define AERON_INTERNAL_PRIMITIVE_COMPACT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct AeronPrimitiveCompactMap {
    uint32_t* source_vertices;
    uint32_t  vertex_count;
    uint32_t* remapped_indices;
    uint32_t  index_count;
} AeronPrimitiveCompactMap;

static inline void AeronPrimitiveCompact_Free(AeronPrimitiveCompactMap* map)
{
    if (!map)
        return;
    free(map->source_vertices);
    free(map->remapped_indices);
    map->source_vertices = NULL;
    map->remapped_indices = NULL;
    map->vertex_count = 0;
    map->index_count = 0;
}

/* Build the primitive-local vertex domain actually addressed by its index
 * stream. source_indices == NULL represents a non-indexed primitive whose
 * implicit stream is 0..index_count-1. First-reference order is stable.
 *
 * The map is deliberately primitive-local: a source vertex referenced by two
 * primitives appears once in each map so consumers can retain per-primitive
 * state such as AeronGltfVertex.prim_id. */
static inline bool AeronPrimitiveCompact_Build(uint32_t source_vertex_count,
                                                const uint32_t* source_indices,
                                                uint32_t index_count,
                                                AeronPrimitiveCompactMap* out)
{
    if (!out)
        return false;
    *out = (AeronPrimitiveCompactMap){0};
    if (!source_vertex_count || !index_count)
        return false;
    if (!source_indices && index_count > source_vertex_count)
        return false;

    uint32_t* source_to_compact =
        (uint32_t*)malloc((size_t)source_vertex_count * sizeof *source_to_compact);
    uint32_t* source_vertices =
        (uint32_t*)malloc((size_t)index_count * sizeof *source_vertices);
    uint32_t* remapped_indices =
        (uint32_t*)malloc((size_t)index_count * sizeof *remapped_indices);
    if (!source_to_compact || !source_vertices || !remapped_indices) {
        free(source_to_compact);
        free(source_vertices);
        free(remapped_indices);
        return false;
    }

    for (uint32_t i = 0; i < source_vertex_count; ++i)
        source_to_compact[i] = UINT32_MAX;

    uint32_t compact_count = 0;
    for (uint32_t i = 0; i < index_count; ++i) {
        const uint32_t source_index = source_indices ? source_indices[i] : i;
        if (source_index >= source_vertex_count) {
            free(source_to_compact);
            free(source_vertices);
            free(remapped_indices);
            return false;
        }
        uint32_t compact_index = source_to_compact[source_index];
        if (compact_index == UINT32_MAX) {
            compact_index = compact_count++;
            source_to_compact[source_index] = compact_index;
            source_vertices[compact_index] = source_index;
        }
        remapped_indices[i] = compact_index;
    }

    free(source_to_compact);
    out->source_vertices = source_vertices;
    out->vertex_count = compact_count;
    out->remapped_indices = remapped_indices;
    out->index_count = index_count;
    return true;
}

#endif /* AERON_INTERNAL_PRIMITIVE_COMPACT_H */

#include "rwt.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

struct rwt_tileset_s {
    int ncolors;
    int ntiles;
    int* ptcounts;
    float** tiles;
    unsigned char* storage;
};

rwt_tileset* rwt_tileset_from_filepath(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    assert(f && "Unable to open tileset file.");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    char* buffer = malloc(fsize);
    fseek(f, 0, SEEK_SET);
    fread(buffer, fsize, 1, f);
    fclose(f);
    rwt_tileset* retval = rwt_tileset_from_memory(buffer, fsize);
    free(buffer);
    return retval;
}

rwt_tileset* rwt_tileset_from_memory(const char* data, int nbytes)
{
    rwt_tileset* retval = malloc(sizeof(struct rwt_tileset_s));
    retval->storage = malloc(nbytes);
    memcpy(retval->storage, data, nbytes);
    int32_t* intdata = (int32_t*) retval->storage;
    retval->ncolors = *intdata++;
    retval->ntiles = *intdata++;
    retval->ptcounts = malloc(sizeof(int32_t) * retval->ntiles);
    retval->tiles = malloc(sizeof(float*) * retval->ntiles);
    for (int i = 0; i < retval->ntiles; i++) {
        intdata++;
        retval->ptcounts[i] = *intdata++;
        float* xy = (float*) intdata;
        retval->tiles[i] = xy;
        intdata += retval->ptcounts[i] * 2;
    }
    assert(intdata == nbytes / 4 + (int*) retval->storage);
    return retval;
}

void rwt_tileset_free(rwt_tileset* tileset)
{
    free(tileset->ptcounts);
    free(tileset->tiles);
    free(tileset->storage);
    free(tileset);
}

void rwt_tileset_info(rwt_tileset* tiles, int* ncolors, int* ntiles)
{
    *ncolors = tiles->ncolors;
    *ntiles = tiles->ntiles;
}

float* rwt_get_sequence(rwt_tileset* tiles, int tile_index, int* npoints)
{
    assert(tile_index >= 0 && tile_index <= tiles->ntiles);
    *npoints = tiles->ptcounts[tile_index];
    return tiles->tiles[tile_index];
}

unsigned int rwt_get_colorcode(rwt_tileset* tiles, int tile_index)
{
    assert(tile_index >= 0 && tile_index <= tiles->ntiles);
    int* ptr = (int*) tiles->tiles[tile_index];
    return *(ptr - 2);
}

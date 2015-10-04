#include "rwt.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct rwt_tileset_s {
    int ntiles;
    int* ptcounts;
    float* tiles;
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
    rwt_tileset* retval = rwt_tileset_from_data(buffer, fsize);
    free(buffer);
    return retval;
}

rwt_tileset* rwt_tileset_from_data(const char* data, int nbytes)
{
    rwt_tileset* retval = malloc(sizeof(struct rwt_tileset_s));
    return retval;
}

void rwt_tileset_free(rwt_tileset* tileset)
{
    free(tileset);
}

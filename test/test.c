#include "../rwt.h"
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char** argv)
{
    rwt_tileset* tiles = rwt_tileset_from_filepath("wangtiles.bin");
    assert(tiles);

    int ncolors, ntiles;
    rwt_tileset_info(tiles, &ncolors, &ntiles);
    assert(ncolors == 3 && ntiles == 81);

    int npoints;
    float* ptdata = rwt_get_sequence(tiles, 0, &npoints);
    assert(ptdata && npoints > 0);

    unsigned int ccode = rwt_get_colorcode(tiles, 0);
    assert(ccode == 0);

    unsigned char* imagedata;
    unsigned width, height;
    unsigned err = lodepng_decode_file(&imagedata, &width, &height,
        "test/butterflies.png", LCT_GREY, 8);
    assert(err == 0 && width == 1275 && height == 1650);

    free(imagedata);
    rwt_tileset_free(tiles);
    return 0;
}

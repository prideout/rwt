#include "../rwt.h"
#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char** argv)
{
    rwt_tileset* tiles = rwt_tileset_from_filepath("wangtiles.bin");
    assert(tiles);

    unsigned char* imagedata;
    unsigned width, height;
    unsigned err = lodepng_decode_file(&imagedata, &width, &height,
        "test/butterflies.png", LCT_GREY, 8);
    assert(err == 0 && width == 1275 && height == 1650);

    free(imagedata);
    rwt_tileset_free(tiles);
    return 0;
}

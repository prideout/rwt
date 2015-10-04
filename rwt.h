#pragma once

typedef struct rwt_tileset_s rwt_tileset;

// Load the binary file at the given path.
rwt_tileset* rwt_tileset_from_filepath(const char* path);

// Load the binary file from memory.
rwt_tileset* rwt_tileset_from_memory(const char* data, int nbytes);

// Free all memory associated with the given tile set.
void rwt_tileset_free(rwt_tileset*);

// Get information about tiles.  Note that ntiles should always be ncolors^4.
void rwt_tileset_info(rwt_tileset* tiles, int* ncolors, int* ntiles);

// Get a pointer to the XY data for a given tile and obtain the number of
// points in the tile.  The XY data is owned by the tile set.
float* rwt_get_sequence(rwt_tileset* tiles, int tile_index, int* npoints);

// Get a packed 4-byte color code for the edges of the given tile.
unsigned int rwt_get_colorcode(rwt_tileset* tiles, int tile_index);

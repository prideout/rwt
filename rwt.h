#pragma once

typedef struct rwt_tileset_s rwt_tileset;

// Load the binary file at the given path.
rwt_tileset* rwt_tileset_from_filepath(const char* path);

// Load the binary file from memory.
rwt_tileset* rwt_tileset_from_data(const char* data, int nbytes);

// Free all memory associated with the given tile set.
void rwt_tileset_free(rwt_tileset*);

#ifndef CHUNK_GENERATION_H
#define CHUNK_GENERATION_H

typedef struct ChunkAssetFile ChunkAssetFile;
ChunkAssetFile* generate_chunk_asset_file(ChunkType type);

void set_generation_enabled(bool state);
bool is_generation_enabled();

#endif  // CHUNK_GENERATION_H
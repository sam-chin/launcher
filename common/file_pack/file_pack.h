#ifndef FILE_PACK_H
#define FILE_PACK_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACK_MAGIC 0x5041434B
#define PACK_VERSION 0x00010000

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t file_count;
    uint32_t crc32;
} PackHeader;

typedef struct {
    uint32_t file_length;
    uint16_t path_length;
} FileEntryHeader;
#pragma pack(pop)

typedef struct {
    char* relative_path;
    uint8_t* file_data;
    size_t file_size;
} FileEntry;

typedef struct {
    FileEntry* entries;
    size_t entry_count;
    size_t capacity;
} FilePack;

FilePack* file_pack_create();
void file_pack_destroy(FilePack* pack);

int file_pack_add_file(FilePack* pack, const char* file_path, const char* base_dir);
int file_pack_add_directory(FilePack* pack, const char* dir_path);

uint8_t* file_pack_pack(FilePack* pack, size_t* out_size);
FilePack* file_pack_unpack(const uint8_t* data, size_t size);

int file_pack_extract(FilePack* pack, const char* output_dir);

#ifdef __cplusplus
}
#endif

#endif

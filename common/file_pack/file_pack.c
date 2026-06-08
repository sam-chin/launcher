#include "file_pack.h"
#include "../crc32/crc32.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>

static char* get_relative_path(const char* full_path, const char* base_dir) {
    size_t base_len = strlen(base_dir);
    if (strncmp(full_path, base_dir, base_len) != 0) {
        return _strdup(full_path);
    }
    
    const char* relative = full_path + base_len;
    while (*relative == '\\' || *relative == '/') {
        relative++;
    }
    
    char* result = _strdup(relative);
    for (char* p = result; *p; p++) {
        if (*p == '\\') {
            *p = '/';
        }
    }
    return result;
}

static uint8_t* read_file(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t* data = (uint8_t*)malloc(size);
    if (!data) {
        fclose(f);
        return NULL;
    }
    
    size_t read = fread(data, 1, size, f);
    fclose(f);
    
    if (read != (size_t)size) {
        free(data);
        return NULL;
    }
    
    *out_size = size;
    return data;
}

static int write_file(const char* path, const uint8_t* data, size_t size) {
    char dir[MAX_PATH];
    strcpy_s(dir, MAX_PATH, path);
    char* last_slash = strrchr(dir, '\\');
    if (last_slash) {
        *last_slash = '\0';
        CreateDirectoryA(dir, NULL);
    }
    
    FILE* f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    
    return written == size;
}

FilePack* file_pack_create() {
    FilePack* pack = (FilePack*)malloc(sizeof(FilePack));
    if (!pack) {
        return NULL;
    }
    
    pack->capacity = 8;
    pack->entry_count = 0;
    pack->entries = (FileEntry*)calloc(pack->capacity, sizeof(FileEntry));
    
    if (!pack->entries) {
        free(pack);
        return NULL;
    }
    
    return pack;
}

void file_pack_destroy(FilePack* pack) {
    if (!pack) {
        return;
    }
    
    for (size_t i = 0; i < pack->entry_count; i++) {
        free(pack->entries[i].relative_path);
        free(pack->entries[i].file_data);
    }
    
    free(pack->entries);
    free(pack);
}

int file_pack_add_file(FilePack* pack, const char* file_path, const char* base_dir) {
    if (!pack || !file_path) {
        return 0;
    }
    
    if (pack->entry_count >= pack->capacity) {
        size_t new_cap = pack->capacity * 2;
        FileEntry* new_entries = (FileEntry*)realloc(pack->entries, new_cap * sizeof(FileEntry));
        if (!new_entries) {
            return 0;
        }
        pack->entries = new_entries;
        pack->capacity = new_cap;
    }
    
    size_t file_size;
    uint8_t* file_data = read_file(file_path, &file_size);
    if (!file_data) {
        return 0;
    }
    
    char* rel_path = get_relative_path(file_path, base_dir ? base_dir : "");
    if (!rel_path) {
        free(file_data);
        return 0;
    }
    
    pack->entries[pack->entry_count].relative_path = rel_path;
    pack->entries[pack->entry_count].file_data = file_data;
    pack->entries[pack->entry_count].file_size = file_size;
    pack->entry_count++;
    
    return 1;
}

static int add_directory_recursive(FilePack* pack, const char* dir_path, const char* base_dir) {
    char search_path[MAX_PATH];
    sprintf_s(search_path, MAX_PATH, "%s\\*", dir_path);
    
    WIN32_FIND_DATAA find_data;
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }
    
    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH];
        sprintf_s(full_path, MAX_PATH, "%s\\%s", dir_path, find_data.cFileName);
        
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            add_directory_recursive(pack, full_path, base_dir);
        } else {
            file_pack_add_file(pack, full_path, base_dir);
        }
    } while (FindNextFileA(hFind, &find_data));
    
    FindClose(hFind);
    return 1;
}

int file_pack_add_directory(FilePack* pack, const char* dir_path) {
    return add_directory_recursive(pack, dir_path, dir_path);
}

uint8_t* file_pack_pack(FilePack* pack, size_t* out_size) {
    if (!pack || !out_size) {
        return NULL;
    }
    
    size_t total_size = sizeof(PackHeader);
    for (size_t i = 0; i < pack->entry_count; i++) {
        total_size += sizeof(FileEntryHeader);
        total_size += strlen(pack->entries[i].relative_path);
        total_size += pack->entries[i].file_size;
    }
    
    uint8_t* buffer = (uint8_t*)malloc(total_size);
    if (!buffer) {
        return NULL;
    }
    
    uint8_t* ptr = buffer;
    
    PackHeader* header = (PackHeader*)ptr;
    header->magic = PACK_MAGIC;
    header->version = PACK_VERSION;
    header->file_count = (uint32_t)pack->entry_count;
    header->crc32 = 0;
    ptr += sizeof(PackHeader);
    
    for (size_t i = 0; i < pack->entry_count; i++) {
        FileEntryHeader* entry_header = (FileEntryHeader*)ptr;
        entry_header->file_length = (uint32_t)pack->entries[i].file_size;
        entry_header->path_length = (uint16_t)strlen(pack->entries[i].relative_path);
        ptr += sizeof(FileEntryHeader);
        
        memcpy(ptr, pack->entries[i].relative_path, entry_header->path_length);
        ptr += entry_header->path_length;
        
        memcpy(ptr, pack->entries[i].file_data, pack->entries[i].file_size);
        ptr += pack->entries[i].file_size;
    }
    
    header->crc32 = crc32(buffer, total_size);
    *out_size = total_size;
    
    return buffer;
}

FilePack* file_pack_unpack(const uint8_t* data, size_t size) {
    if (!data || size < sizeof(PackHeader)) {
        return NULL;
    }
    
    const uint8_t* ptr = data;
    
    const PackHeader* header = (const PackHeader*)ptr;
    if (header->magic != PACK_MAGIC || header->version != PACK_VERSION) {
        return NULL;
    }
    
    uint32_t expected_crc = header->crc32;
    PackHeader temp_header = *header;
    temp_header.crc32 = 0;
    uint8_t* temp_buffer = (uint8_t*)malloc(size);
    if (!temp_buffer) {
        return NULL;
    }
    memcpy(temp_buffer, data, size);
    memcpy(temp_buffer, &temp_header, sizeof(temp_header));
    uint32_t actual_crc = crc32(temp_buffer, size);
    free(temp_buffer);
    
    if (expected_crc != actual_crc) {
        return NULL;
    }
    
    ptr += sizeof(PackHeader);
    
    FilePack* pack = file_pack_create();
    if (!pack) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < header->file_count; i++) {
        if ((size_t)(ptr - data) + sizeof(FileEntryHeader) > size) {
            file_pack_destroy(pack);
            return NULL;
        }
        
        const FileEntryHeader* entry_header = (const FileEntryHeader*)ptr;
        ptr += sizeof(FileEntryHeader);
        
        if ((size_t)(ptr - data) + entry_header->path_length + entry_header->file_length > size) {
            file_pack_destroy(pack);
            return NULL;
        }
        
        char* rel_path = (char*)malloc(entry_header->path_length + 1);
        if (!rel_path) {
            file_pack_destroy(pack);
            return NULL;
        }
        memcpy(rel_path, ptr, entry_header->path_length);
        rel_path[entry_header->path_length] = '\0';
        ptr += entry_header->path_length;
        
        uint8_t* file_data = (uint8_t*)malloc(entry_header->file_length);
        if (!file_data) {
            free(rel_path);
            file_pack_destroy(pack);
            return NULL;
        }
        memcpy(file_data, ptr, entry_header->file_length);
        ptr += entry_header->file_length;
        
        if (pack->entry_count >= pack->capacity) {
            size_t new_cap = pack->capacity * 2;
            FileEntry* new_entries = (FileEntry*)realloc(pack->entries, new_cap * sizeof(FileEntry));
            if (!new_entries) {
                free(rel_path);
                free(file_data);
                file_pack_destroy(pack);
                return NULL;
            }
            pack->entries = new_entries;
            pack->capacity = new_cap;
        }
        
        pack->entries[pack->entry_count].relative_path = rel_path;
        pack->entries[pack->entry_count].file_data = file_data;
        pack->entries[pack->entry_count].file_size = entry_header->file_length;
        pack->entry_count++;
    }
    
    return pack;
}

int file_pack_extract(FilePack* pack, const char* output_dir) {
    if (!pack || !output_dir) {
        return 0;
    }
    
    for (size_t i = 0; i < pack->entry_count; i++) {
        char full_path[MAX_PATH];
        sprintf_s(full_path, MAX_PATH, "%s\\%s", output_dir, pack->entries[i].relative_path);
        
        for (char* p = full_path; *p; p++) {
            if (*p == '/') {
                *p = '\\';
            }
        }
        
        if (!write_file(full_path, pack->entries[i].file_data, pack->entries[i].file_size)) {
            return 0;
        }
    }
    
    return 1;
}

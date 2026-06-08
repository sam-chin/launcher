#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../common/file_pack/file_pack.h"
#include "../common/encrypt/encrypt.h"

static int is_directory(const char* path) {
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

static void print_usage(const char* prog_name) {
    printf("Usage:\n");
    printf("  Interactive mode: %s\n", prog_name);
    printf("  Command mode: %s <source> <output> <key>\n", prog_name);
    printf("\nExamples:\n");
    printf("  %s\n", prog_name);
    printf("  %s patch_dir patch.pack mysecretkey\n", prog_name);
}

static int encrypt_and_save(const char* source_path, const char* output_path, const char* key) {
    printf("Processing...\n");
    
    FilePack* pack = file_pack_create();
    if (!pack) {
        printf("Error: Cannot create pack object\n");
        return 0;
    }
    
    if (is_directory(source_path)) {
        printf("Adding directory: %s\n", source_path);
        if (!file_pack_add_directory(pack, source_path)) {
            printf("Warning: Some files may fail to add\n");
        }
    } else {
        printf("Adding file: %s\n", source_path);
        char base_dir[MAX_PATH];
        strcpy_s(base_dir, MAX_PATH, source_path);
        char* last_slash = strrchr(base_dir, '\\');
        if (last_slash) {
            *last_slash = '\0';
        } else {
            base_dir[0] = '\0';
        }
        if (!file_pack_add_file(pack, source_path, base_dir)) {
            printf("Error: Cannot add file\n");
            file_pack_destroy(pack);
            return 0;
        }
    }
    
    if (pack->entry_count == 0) {
        printf("Error: No files found\n");
        file_pack_destroy(pack);
        return 0;
    }
    
    printf("File count: %zu\n", pack->entry_count);
    
    size_t pack_size;
    uint8_t* pack_data = file_pack_pack(pack, &pack_size);
    file_pack_destroy(pack);
    
    if (!pack_data) {
        printf("Error: Packing failed\n");
        return 0;
    }
    
    printf("Pack size: %zu bytes\n", pack_size);
    
    printf("Encrypting...\n");
    xor_encrypt(pack_data, pack_size, (const uint8_t*)key, strlen(key));
    
    FILE* f = fopen(output_path, "wb");
    if (!f) {
        printf("Error: Cannot create output file\n");
        free(pack_data);
        return 0;
    }
    
    size_t written = fwrite(pack_data, 1, pack_size, f);
    fclose(f);
    free(pack_data);
    
    if (written != pack_size) {
        printf("Error: Failed to write file\n");
        return 0;
    }
    
    printf("Success!\n");
    printf("Output file: %s\n", output_path);
    return 1;
}

static void interactive_mode() {
    char source_path[MAX_PATH];
    char output_path[MAX_PATH];
    char key[256];
    
    printf("=== Encrypt Tool ===\n\n");
    
    printf("Please enter source path (file or directory): ");
    fgets(source_path, MAX_PATH, stdin);
    source_path[strcspn(source_path, "\r\n")] = '\0';
    
    printf("Please enter output file path: ");
    fgets(output_path, MAX_PATH, stdin);
    output_path[strcspn(output_path, "\r\n")] = '\0';
    
    printf("Please enter encryption key: ");
    fgets(key, 256, stdin);
    key[strcspn(key, "\r\n")] = '\0';
    
    printf("\n");
    encrypt_and_save(source_path, output_path, key);
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        interactive_mode();
    } else if (argc == 4) {
        if (!encrypt_and_save(argv[1], argv[2], argv[3])) {
            return 1;
        }
    } else {
        print_usage(argv[0]);
        return 1;
    }
    
    return 0;
}

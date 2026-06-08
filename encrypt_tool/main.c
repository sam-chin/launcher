#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../common/file_pack/file_pack.h"
#include "../common/encrypt/encrypt.h"

static void set_console_encoding() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
}

static int is_directory(const char* path) {
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

static void print_usage(const char* prog_name) {
    printf("用法:\n");
    printf("  交互式模式: %s\n", prog_name);
    printf("  命令行模式: %s <源路径> <输出文件> <密钥>\n", prog_name);
    printf("\n示例:\n");
    printf("  %s\n", prog_name);
    printf("  %s patch_dir patch.pack mysecretkey\n", prog_name);
}

static int encrypt_and_save(const char* source_path, const char* output_path, const char* key) {
    printf("正在处理...\n");
    
    FilePack* pack = file_pack_create();
    if (!pack) {
        printf("错误: 无法创建打包对象\n");
        return 0;
    }
    
    if (is_directory(source_path)) {
        printf("添加目录: %s\n", source_path);
        if (!file_pack_add_directory(pack, source_path)) {
            printf("警告: 部分文件可能添加失败\n");
        }
    } else {
        printf("添加文件: %s\n", source_path);
        char base_dir[MAX_PATH];
        strcpy_s(base_dir, MAX_PATH, source_path);
        char* last_slash = strrchr(base_dir, '\\');
        if (last_slash) {
            *last_slash = '\0';
        } else {
            base_dir[0] = '\0';
        }
        if (!file_pack_add_file(pack, source_path, base_dir)) {
            printf("错误: 无法添加文件\n");
            file_pack_destroy(pack);
            return 0;
        }
    }
    
    if (pack->entry_count == 0) {
        printf("错误: 没有找到文件\n");
        file_pack_destroy(pack);
        return 0;
    }
    
    printf("文件数量: %zu\n", pack->entry_count);
    
    size_t pack_size;
    uint8_t* pack_data = file_pack_pack(pack, &pack_size);
    file_pack_destroy(pack);
    
    if (!pack_data) {
        printf("错误: 打包失败\n");
        return 0;
    }
    
    printf("打包大小: %zu 字节\n", pack_size);
    
    printf("正在加密...\n");
    xor_encrypt(pack_data, pack_size, (const uint8_t*)key, strlen(key));
    
    FILE* f = fopen(output_path, "wb");
    if (!f) {
        printf("错误: 无法创建输出文件\n");
        free(pack_data);
        return 0;
    }
    
    size_t written = fwrite(pack_data, 1, pack_size, f);
    fclose(f);
    free(pack_data);
    
    if (written != pack_size) {
        printf("错误: 写入文件失败\n");
        return 0;
    }
    
    printf("成功!\n");
    printf("输出文件: %s\n", output_path);
    return 1;
}

static void interactive_mode() {
    char source_path[MAX_PATH];
    char output_path[MAX_PATH];
    char key[256];
    
    printf("=== 独立补丁加密工具 ===\n\n");
    
    printf("请输入源路径 (文件或目录): ");
    fgets(source_path, MAX_PATH, stdin);
    source_path[strcspn(source_path, "\r\n")] = '\0';
    
    printf("请输入输出文件路径: ");
    fgets(output_path, MAX_PATH, stdin);
    output_path[strcspn(output_path, "\r\n")] = '\0';
    
    printf("请输入加密密钥: ");
    fgets(key, 256, stdin);
    key[strcspn(key, "\r\n")] = '\0';
    
    printf("\n");
    encrypt_and_save(source_path, output_path, key);
}

int main(int argc, char* argv[]) {
    set_console_encoding();
    
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

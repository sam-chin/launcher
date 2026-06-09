#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "../common/file_pack/file_pack.h"
#include "../common/encrypt/encrypt.h"

static void console_out(const char* utf8_text) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8_text, -1, NULL, 0);
    if (len > 0) {
        wchar_t* wbuf = (wchar_t*)malloc(sizeof(wchar_t) * len);
        if (wbuf) {
            MultiByteToWideChar(CP_UTF8, 0, utf8_text, -1, wbuf, len);
            DWORD written;
            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), wbuf, len - 1, &written, NULL);
            free(wbuf);
        }
    }
}

#define PRINT(...) do { \
    char _buf[4096]; \
    snprintf(_buf, sizeof(_buf), __VA_ARGS__); \
    console_out(_buf); \
} while(0)

static int is_directory(const char* path) {
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

static void print_usage(const char* prog_name) {
    PRINT("用法:\n");
    PRINT("  交互模式: %s\n", prog_name);
    PRINT("  命令模式: %s <源路径> <输出文件> <密钥>\n", prog_name);
    PRINT("\n示例:\n");
    PRINT("  %s\n", prog_name);
    PRINT("  %s patch_dir patch.pack mysecretkey\n", prog_name);
}

static int encrypt_and_save(const char* source_path, const char* output_path, const char* key) {
    PRINT("正在处理...\n");

    FilePack* pack = file_pack_create();
    if (!pack) {
        PRINT("错误: 无法创建打包对象\n");
        return 0;
    }

    if (is_directory(source_path)) {
        PRINT("添加目录: %s\n", source_path);
        if (!file_pack_add_directory(pack, source_path)) {
            PRINT("警告: 部分文件可能添加失败\n");
        }
    } else {
        PRINT("添加文件: %s\n", source_path);
        char base_dir[MAX_PATH];
        strcpy_s(base_dir, MAX_PATH, source_path);
        char* last_slash = strrchr(base_dir, '\\');
        if (last_slash) {
            *last_slash = '\0';
        } else {
            base_dir[0] = '\0';
        }
        if (!file_pack_add_file(pack, source_path, base_dir)) {
            PRINT("错误: 无法添加文件\n");
            file_pack_destroy(pack);
            return 0;
        }
    }

    if (pack->entry_count == 0) {
        PRINT("错误: 未找到文件\n");
        file_pack_destroy(pack);
        return 0;
    }

    PRINT("文件数量: %zu\n", pack->entry_count);

    size_t pack_size;
    uint8_t* pack_data = file_pack_pack(pack, &pack_size);
    file_pack_destroy(pack);

    if (!pack_data) {
        PRINT("错误: 打包失败\n");
        return 0;
    }

    PRINT("打包大小: %zu 字节\n", pack_size);

    PRINT("正在加密...\n");
    xor_encrypt(pack_data, pack_size, (const uint8_t*)key, strlen(key));

    FILE* f = NULL;
    if (fopen_s(&f, output_path, "wb") != 0 || !f) {
        PRINT("错误: 无法创建输出文件\n");
        free(pack_data);
        return 0;
    }

    size_t written = fwrite(pack_data, 1, pack_size, f);
    fclose(f);
    free(pack_data);

    if (written != pack_size) {
        PRINT("错误: 文件写入失败\n");
        return 0;
    }

    PRINT("加密完成!\n");
    PRINT("输出文件: %s\n", output_path);
    return 1;
}

static void interactive_mode() {
    char source_path[MAX_PATH];
    char output_path[MAX_PATH];
    char key[256];

    PRINT("=== 加密工具 ===\n\n");

    PRINT("请输入源路径(文件或目录): ");
    fgets(source_path, MAX_PATH, stdin);
    source_path[strcspn(source_path, "\r\n")] = '\0';

    PRINT("请输入输出文件路径: ");
    fgets(output_path, MAX_PATH, stdin);
    output_path[strcspn(output_path, "\r\n")] = '\0';

    PRINT("请输入加密密钥: ");
    fgets(key, 256, stdin);
    key[strcspn(key, "\r\n")] = '\0';

    PRINT("\n");
    encrypt_and_save(source_path, output_path, key);
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);

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

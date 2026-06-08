#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc32_init();
uint32_t crc32_update(uint32_t crc, const void* data, size_t length);
uint32_t crc32_final(uint32_t crc);
uint32_t crc32(const void* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif

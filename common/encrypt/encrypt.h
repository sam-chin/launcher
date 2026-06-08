#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void xor_encrypt(uint8_t* data, size_t length, const uint8_t* key, size_t key_length);
void xor_decrypt(uint8_t* data, size_t length, const uint8_t* key, size_t key_length);

#ifdef __cplusplus
}
#endif

#endif

#include "encrypt.h"

void xor_encrypt(uint8_t* data, size_t length, const uint8_t* key, size_t key_length) {
    if (key_length == 0) {
        return;
    }
    
    for (size_t i = 0; i < length; i++) {
        data[i] ^= key[i % key_length];
    }
}

void xor_decrypt(uint8_t* data, size_t length, const uint8_t* key, size_t key_length) {
    xor_encrypt(data, length, key, key_length);
}

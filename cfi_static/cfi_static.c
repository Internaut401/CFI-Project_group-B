#include <stdio.h>
#include <stdint.h>

const uint8_t key[] = {
    0xEF, 0xBE, 0xAD, 0xDE
};

void cfi_pa_init() {
    //TODO
    return;
}

void *
cfi_pa_encrypt(void *ptr) {
    printf("Prova\n");
    return NULL;
}

void *
cfi_pa_decrypt(void *ptr) {
    printf("Prova decrypt\n");
    return NULL;
}

const uint8_t *
get_key() {
    return key;
}

void prova() {
    printf("Hello from static library\n");
}
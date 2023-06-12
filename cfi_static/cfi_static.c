#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include<sys/ioctl.h>

#define DEVICE_NAME "/dev/mydriver"
#define OVERWRITE_RET_ADDR(new) {\
uintptr_t *__ret_addr = (uintptr_t *) ((void *) __builtin_frame_address(1) + sizeof(uintptr_t));\
*__ret_addr = (new);\
}
#define RET_ADDR (__builtin_extract_return_addr(__builtin_return_address(1)))
#define RET_ADDR_PTR ((uintptr_t *) ((void *) __builtin_frame_address(1) + sizeof(uintptr_t)))

/*
const uint8_t key[] = {
    0xEF, 0xBE, 0xAD, 0xDE
};
*/

static int key_dev;

void cfi_pa_init() {
    printf("Initializing CFI PA\n");
    if ((key_dev = open(DEVICE_NAME, O_RDWR)) < 0) {
        perror("Cannot open QARMA device\n");
        exit(-1);
    }
    ioctl(key_dev, 2, NULL); // Generate key
    return;
}

void
cfi_pa_encrypt() {
    printf("Encrypting\n");
    uintptr_t *ret_ptr = RET_ADDR_PTR;
    ioctl(key_dev, 0, ret_ptr);
}

void
cfi_pa_decrypt() {
    printf("Decrypting\n");
    uintptr_t *ret_ptr = RET_ADDR_PTR;
    ioctl(key_dev, 1, ret_ptr);
}

void cfi_pa_exit() {
    close(key_dev);
    printf("Closed CFI PA\n");
}

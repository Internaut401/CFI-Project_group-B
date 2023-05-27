#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define DEVICE_NAME "/dev/mydriver"

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
    int fd;
    uintptr_t *ptr = 0x4141414142424242;

    printf("Hello from static library\n");


    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
/*
    ptr = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
*/
    ioctl(fd, 0, &ptr); // Saves the integer value in kernel memory

    printf("encrypted pointer value is %p\n", ptr);
    ioctl(fd, 1, &ptr); // Retrieves the integer value from kernel memory

    printf("retrived pointer value is %p\n", ptr);

  //  munmap(ptr, sizeof(int));
    close(fd);
}

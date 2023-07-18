/** cfi_static.c
 * Contains CFI-PA-related functions that are statically linked into the target program and called in 
 * prologues and epilogues of functions. Their role is to communicate with the kernel driver to generate 
 * and check signatures.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/mydriver"
//#define OVERWRITE_RET_ADDR(new) {\
//uintptr_t *__ret_addr = (uintptr_t *) ((void *) __builtin_frame_address(1) + sizeof(uintptr_t));\
//*__ret_addr = (new);\
//}
//#define RET_ADDR (__builtin_extract_return_addr(__builtin_return_address(1)))

// Returns a pointer to the return address of the calling function on the stack
#define RET_ADDR_PTR ((uintptr_t *) ((void *) __builtin_frame_address(1) + sizeof(uintptr_t)))

// File descriptor of the CFI-PA device
static int key_dev = -1;
volatile static int ret = -1;
/** Initialize CFI-PA
 * Opens the QARMA device. If not possible, exit with an error code.
*/

void cfi_pa_init() __attribute__((constructor));
void cfi_pa_exit() __attribute__((destructor));

void cfi_pa_init() {
    if ((key_dev = open(DEVICE_NAME, O_RDWR)) < 0) {
        perror("Cannot open QARMA device\n");
        exit(-1);
    }

    // DISMISS
    ioctl(key_dev, 2, NULL); // Generate key
    return;
}

/** Generate a signed address.
 * 
 * Passes a pointer to the return address on the stack to the driver, which will then read 
 * the address, generate the signature and overwrite it with the signed address.
 */
void
cfi_pa_encrypt() {
    
    // extract return address pointer
    uintptr_t *ret_ptr = RET_ADDR_PTR;
    
    // invoke kernel module to encrypt it
    ioctl(key_dev, 0, ret_ptr);
    __asm__ volatile("pop %%r9\n pop %%r8\n pop %%rsi\n pop %%rdi\n pop %%rdx\n pop %%rcx\n pop %%rax " : : : );
}

/** Check the signature of the return address.
 * 
 * Pass a pointer to the return address on the stack to the driver, which will read the signed 
 * address, check the signature and:
 *   - if the signature is ok, overwrite it with the original address
 *   - if not, overwrite it with NULL
 */
void
cfi_pa_decrypt() {
    uintptr_t *ret_ptr = RET_ADDR_PTR;
    
    // invoke kernel module to authenticate it
    ioctl(key_dev, 1, ret_ptr);
    __asm__ volatile("pop %%r9\n pop %%r8\n pop %%rsi\n pop %%rdi\n pop %%rdx\n pop %%rcx\n pop %%rax " : : : );
}

/** Close the file descriptor.
 */
void cfi_pa_exit() {
    close(key_dev);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define OVERWRITE_RET_ADDR(new) {\
uintptr_t *__ret_addr = (uintptr_t *) ((void *) __builtin_frame_address(0) + sizeof(uintptr_t));\
*__ret_addr = (new);\
}

void fun1(){
    printf("Hello i'm fun1\n");
}

void foo() {
    printf("Should not print\n");
}

void bad_func() {
    printf("I'm evil\n");
    OVERWRITE_RET_ADDR((uintptr_t) &foo | 0x4141000000000000);
}

void good_func() {
    printf("I'm good\n");
}

void main(){
    printf("Hello World!\n");
    fun1();
    good_func();
    //bad_func();
}

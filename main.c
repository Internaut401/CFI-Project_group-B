#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define OVERWRITE_RET_ADDR(new) {\
uintptr_t *__ret_addr = (uintptr_t *) ((void *) __builtin_frame_address(0) + sizeof(uintptr_t));\
*__ret_addr = (new);\
}


void nested_fun(){
	printf("I'm a nested function\n");
}


void fun1(){
    printf("Hello i'm fun1\n");
    nested_fun();
}


void foo() {
    printf("Should not print\n");
}


void bad_func() {
    printf("I'm evil\n");
    OVERWRITE_RET_ADDR((uintptr_t) &foo);
}


void good_func() {
    printf("I'm good\n");
}


void main(){
    printf("Hello World!\n");
    
    fun1();
    printf("fun1() returned successfully\n");

    good_func();
    printf("good_func() returned successfully\n");

    bad_func();
    printf("You shall not print\n");
}

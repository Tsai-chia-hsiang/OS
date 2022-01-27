#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(){

    char *buffer, *ret;
    long int len = 2; 
    
    __asm__ volatile(

        "mov $0, %%rax\n" //sys_read: rax:0 
        "mov $0, %%rdi\n"//rdi:file identity
        "mov %1, %%rsi\n"//rsi: char *buffer
        "mov %2, %%rdx\n"//rdx: buffer size 
        "syscall\n"
        "mov %%rax, %0"
        : "=m"(ret)
        : "m" (buffer), "g" (len)
        : "rax", "rbx", "rcx", "rdx"
    );
    
    printf("讀入的字元為 \"%c\" \n", *buffer);
    return 0;
}

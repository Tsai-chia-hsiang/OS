#include <stdio.h>

int main(int argc, char** argv) {

    unsigned long int msr;
    asm volatile (

        "rdtsc\n\t"  //edx:eax
        "shl $32, %%rdx\n\t" //get higher bit part to right(left) position
        "mov %%rdx, %0\n"   //higher bit part in msr
        "or %%rax, %0\n"    //using msr(higher bit part) or rax which contents lower bits
                            //and store into msr
        : "=m" (msr) 
        :
        : "rdx","rax"
    );
    printf("msr: %lx\n", msr);
}

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#define likely(x)    __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)
long g = 0;

//第2題
static inline int my_spin_lock (atomic_int *lock) {
    int val=0;
    if (likely(atomic_exchange_explicit(lock, 1, memory_order_acq_rel) == 0))        return 0;
    /*如果lock 值是0， atomic_exchange_explicit將lock換為1 後，回傳0（即lock舊值），得到lock，直接回傳。若lock 不是0，則不會回傳0，要進入下面while spin*/
    do {
        do {
            asm("pause"); //讓CPU負擔小一些
        } while (*lock != 0);
	//基於相信硬體cache coherence，直接判斷lock是不是0。是0代表沒有人有lock，進入下面	atomic來競爭。

        val = 0; //except value

    } while (!atomic_compare_exchange_weak_explicit(lock, &val, 1, memory_order_acq_rel, memory_order_relaxed));
	//最先執行到 atomic_compare_exchange_weak_explicit的process將lock 改為1 ，該函數回傳true(lock == val)。之後執行到的人都回傳false而回到while繼續等待。

    return 0;
	//拿到lock
}


static inline int my_spin_unlock(atomic_int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
    //將lock改為0，即unlock
    return 0;
}


atomic_int a_lock;
atomic_long count_array[256];
int numCPU;

void sigHandler(int signo) {
    for (int i=0; i<numCPU; i++) {
        printf("%i, %ld\n", i, count_array[i]);
    }

    exit(0);
}

atomic_int in_cs=0;
atomic_int wait=1;

//第3題
void thread(void *givenName) {
    int givenID = (intptr_t)givenName; //給予一個流水號的id 編號
    srand((unsigned)time(NULL)); //設定random seed (下面模擬sleep要用)


    unsigned int rand_seq;
    cpu_set_t set; CPU_ZERO(&set); CPU_SET(givenID, &set);
    sched_setaffinity(gettid(), sizeof(set), &set);
    //第i個thread 在第i 個core上執行

    while(atomic_load_explicit(&wait, memory_order_acquire));
    //用wait(初始話給1)來等待所有的thread都建立好（讓wait 變0）（在主程式裡面實現）在執行以下程	式

    while(1) {
        my_spin_lock(&a_lock); //去要lock

        atomic_fetch_add(&in_cs, 1); //lock要到，進入CS，將global 的in_cs(有多少process 在CS)  +1

        atomic_fetch_add_explicit(&count_array[givenID], 1, memory_order_relaxed);
	    //把自己進入CS的次數+1

        if (in_cs != 1) {
	        //如果同時有一個以上的process在更改CS，違反mutual exclusive，造成race condition，致命錯誤，直接結束程式
            printf("violation: mutual exclusion\n");
            exit(0);
        }
        atomic_fetch_add(&in_cs, -1);  //離開CS, in_cs -1 

        my_spin_unlock(&a_lock); //unlock 這個CS的lock

        int delay_size = rand_r(&rand_seq)%73; //sleep time, max = 73(隨便取的)

        for (int i=0; i<delay_size; i++){ //模擬sleep
            g++; //為了不要讓-O3把這個for優化掉，讓它做一些事情，但我的gcc好像還是把它優化掉了QQ
        }
            
            
    }
}



int main(int argc, char **argv) {
    signal(SIGALRM, sigHandler);
    alarm(5);
    numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);

    for (long i=0; i< numCPU; i++)
        pthread_create(&tid[i],NULL,(void *) thread, (void*)i);
    atomic_store(&wait,0);

    for (int i=0; i< numCPU; i++)
        pthread_join(tid[i],NULL);
    
   
}

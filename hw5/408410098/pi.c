#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>
#include <math.h>
//double ans = 3.1415926535;


typedef struct thread_data{
    long int id;
    long int x_start_idx;
    long int x_end_idx;   

} thread_data;



double local_y_sum[100];
double r = 2;
long int partition_num = 0;
double x_unit = 0;


void thread(void* arg){

    thread_data *data = (thread_data *)arg;
   // printf("%ld --> %ld\n", data->x_start_idx, data->x_end_idx);
    
    double x_coordinate = 0.0;
    double y_coordinate = 0.0;
    for(long int i = data->x_start_idx; i<data->x_end_idx; i++){
        x_coordinate = (double)(i)*x_unit; 
        y_coordinate = sqrt((double)4 - x_coordinate*x_coordinate);
        local_y_sum[data->id]+= y_coordinate;
    }
      
}


void compare_pi(double upper, double lower, int precise){

    char upperbuffer[13];
    char lowerbuffer[13];
    char ans_buffer[precise+3];

    sprintf(upperbuffer, "%.10f", upper);
    sprintf(lowerbuffer, "%.10f", lower);

    for(int i = 0; i<precise+2;i++){
        if(upperbuffer[i] != lowerbuffer[i]){
            printf("Can't reach the accuracy of the requirtment(to decimal %d places)\n", precise);
            printf("Current pi is accurate to %d decimal place:\n",i-2);
            strncpy(ans_buffer, upperbuffer,i-1);
            printf("lowerbound = %s, upperbound = %s\n", lowerbuffer, upperbuffer);
            return;
        }
    }
    strncpy(ans_buffer, upperbuffer,precise+2);
    printf("pi in accuracy to %d decimal places: %s\n",precise, ans_buffer);
}



int main(int argc, char** argv){
    
    if(argc < 2){
        printf("should give a int number to argv[1] (ex: ./pi 6) as pi's accuracy.\n"); 
        exit(0);
    }
    
    int precise = atoi(argv[1]);
    int numCPU = -1;

    if (argc < 3){
        printf("Using defualt thread number : cpu core number\n");
        numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    }else{
        numCPU = atoi(argv[2]);
    }
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);
    thread_data* arg = (thread_data*)malloc(sizeof(thread_data)* numCPU);

    
    partition_num = 1000000000;
    double thread_responding_x = (double)partition_num/(double)numCPU;
    printf("Each thread is responsible for %.10f units\n",thread_responding_x); 
    x_unit = r/(double)partition_num;

    printf("waiting for %d child threads\n",numCPU);

    for (long int i=0; i< numCPU; i++){
        (arg+i)->id = i;
        (arg+i)->x_start_idx = thread_responding_x*i;
        (arg+i)->x_end_idx = thread_responding_x*(i+1);

        pthread_create(&tid[i],NULL,(void *) thread,(void *)(arg+i));
    }

    for (int i=0; i< numCPU; i++){
	    pthread_join(tid[i],NULL);
    }
    printf("all threads finish their work\n\n");

    double upperpi = 0.0;
    for (int i = 0;i<numCPU; i++){
        upperpi += local_y_sum[i];
    }
    upperpi *= x_unit;
    //all rectangle shift left. Minusing extra area(x coordinate = 0, y = 2) 
    double lowerpi = upperpi - x_unit*(double)2.00;
    compare_pi(upperpi, lowerpi, precise);
    return 0;
}

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include "utils.h"
#include <unistd.h>

#define SIZE 4096

int arr[SIZE];
uint64_t start_time;
volatile int start = 0;
volatile uint64_t times[2];

void* p_write(void* arg)
{
	int val = *((int*)arg);
	int N;
	int vals[SIZE];
	int *p=arr;
	uint64_t time;
	// Wait for start
	while(!start)
		;
	// Get current time
    asm volatile("mfence");
    asm volatile("rdtsc"
                 : "=a"(time));
    asm volatile("mfence");
	// Start
	for(int i=0;i<SIZE;++i,++p){
		*p=val;
	}
	// Save time
	times[val]=time;
    return NULL;
}

int main()
{

	// Threads and results
    pthread_t threads[2];
	//uint64_t start,end,last=0;
	int a = 1, b = 0;
	int i=0;
	uint64_t time_diff;

	do{
		// Create thread
		pthread_create(&threads[0], NULL, p_write, &a);
		pthread_create(&threads[1], NULL, p_write, &b);
		set_thread_cpu(threads[0],5);
		set_thread_cpu(threads[1],6);
		
		// Wait for finish
		start=1;
		pthread_join(threads[0], NULL);
		pthread_join(threads[1], NULL);
		start=0;
		time_diff = (times[1]>times[0])?(times[1]-times[0]):(times[0]-times[1]);
		++i;
	}while(time_diff>50);
	printf("Thread %d Start time %lu\n",0,times[0]);
	printf("Thread %d Start time %lu\n",1,times[1]);
	printf("Time diff %lu\n",time_diff);
	printf("Total execution %d\n",i);
	printf("Result:\n");
	for(int i=0;i<SIZE;++i){
		printf("%d ",arr[i]);
	}
	printf("\n");
	return 0;
}
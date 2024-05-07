#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include "utils.h"
#include <unistd.h>

// Shared array size
#define ARRAY_SIZE 512

// Start time difference threshold
uint64_t max_time_diff = 100;
uint64_t min_time_diff = 30;

int arr[ARRAY_SIZE];
uint64_t start_time;
volatile int start = 0;
volatile uint64_t times[2];

void* p_write(void* arg)
{
	int val = *((int*)arg);
	int N;
	int vals[ARRAY_SIZE];
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
	for(int i=0;i<ARRAY_SIZE;++i,++p){
		*p=val;
	}
	// Save time
	times[val]=time;
    return NULL;
}

int main(int argc, char *argv[])
{

	// Threads and results
    pthread_t threads[2];
	//uint64_t start,end,last=0;
	int a = 1, b = 0;
	int i=0;
	uint64_t time_diff;

	int opt;
	while ((opt = getopt(argc, argv, "hm:n:")) != -1) {
		switch (opt) {
		case 'm':
			max_time_diff = atoi(optarg);
			break;
		case 'n':
			min_time_diff = atoi(optarg);
			break;
		case 'h':
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-m max_time_diff] [-n min_time_diff]\n",
					argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if(min_time_diff>max_time_diff){
		fprintf(stderr, "Usage: %s [-m max_time_diff] [-n min_time_diff]\n",
				argv[0]);
		exit(EXIT_FAILURE);	
	}

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
	}while(!(time_diff>=min_time_diff && time_diff<=max_time_diff));

	printf("Thread %d Start time %lu\n",0,times[0]);
	printf("Thread %d Start time %lu\n",1,times[1]);
	printf("Start time difference range (%lu,%lu)\n",max_time_diff,min_time_diff);
	printf("Time diff %lu\n",time_diff);
	printf("Total execution %d\n",i);
	printf("Result:\n");
	for(int i=0;i<ARRAY_SIZE;++i){
		printf("%d ",arr[i]);
	}
	printf("\n");
	return 0;
}
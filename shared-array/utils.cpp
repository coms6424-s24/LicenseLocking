#include "utils.h"
#include <sched.h>
#include <stdio.h>
#include <unistd.h>

void set_cpu(int cpu_id)
{
    cpu_set_t mask;
    int status;

    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    status = sched_setaffinity(getpid(), sizeof(mask), &mask);
    if (status != 0) {
        perror("sched_setaffinity");
    }
}

void set_thread_cpu(pthread_t thread, int cpu_id)
{
    cpu_set_t mask;
    int status;

    CPU_ZERO(&mask);
    CPU_SET(cpu_id, &mask);
    status = pthread_setaffinity_np(thread, sizeof(mask), &mask);
    if (status != 0) {
        perror("pthread_setaffinity_np");
    }
}
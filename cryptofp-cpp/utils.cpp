#include "utils.h"
#include <unistd.h>
#include <sched.h>
#include <stdio.h>

void set_cpu(int cpu_id)
{
	cpu_set_t mask;
	int status;

	CPU_ZERO(&mask);
	CPU_SET(cpu_id, &mask);
	status = sched_setaffinity(getpid(), sizeof(mask), &mask);
	if (status != 0){
		perror("sched_setaffinity");
	}
}
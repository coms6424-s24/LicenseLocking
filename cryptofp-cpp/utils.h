#ifndef __UTILS_H__
#define __UTILS_H__

#include <array>
#include <pthread.h>

// page 5, section 3.3.3
constexpr int n = 1000;
constexpr int m = 50;

void set_cpu(int cpu_id);
void set_thread_cpu(pthread_t thread,int cpu_id);

#endif

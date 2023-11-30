#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define DEBUG_THIS_MODULE                             0
struct test_params {
    void* addr;
};

#define RUNNING_CYCLE_LIMITS     1000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
int g_init = 0;

void* task1(void* arg)
{
    struct test_params * p_param = (struct test_params *)arg;
    void* bar = p_param->addr;
    int* val0_ref = bar;
    int* val1_ref = val0_ref + 1;
    int val0;
    int val1;
    int count = 0;
    while(1)
	{
        /**
        ** This is the code for EP0, EP0 starts first
        */
        // EP0 should stop here until EP1 runs
        // increase val0
        val0 = *val0_ref = val0 + 1;
#if DEBUG_THIS_MODULE
        printf("[Before EP0] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        while(*val1_ref != val1 + 1);
        val1 = val1 + 1;
#if DEBUG_THIS_MODULE
        printf("[After EP0] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        if(++count >= RUNNING_CYCLE_LIMITS) {
            break;
        }
	}
    printf("[DEBUG] Task 1 left \r\n");
    return NULL;
}


void* task2(void* arg)
{
    struct test_params * p_param = (struct test_params *)arg;
    void* bar = p_param->addr;
    int* val0_ref = bar;
    int* val1_ref = val0_ref + 1;
    int val0;
    int val1;
    int count = 0;
    while(1){
        /**
        ** This is the code for EP1, EP1 starts later
        */
#if DEBUG_THIS_MODULE
        printf("[Before EP1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        // EP1 should stop here until EP0 runs
        while(*val0_ref != val0 + 1);
        // increase val1 first, so EP0 can continue to run
        val1 = *val1_ref = val1 + 1;
        val0 = val0 + 1;
#if DEBUG_THIS_MODULE
        printf("[EP1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        if(++count >= RUNNING_CYCLE_LIMITS) {
            break;
        }
	}
    printf("[DEBUG] Task 2 left \r\n");
	return NULL;
}

int main()
{
    pthread_t thread1, thread2;
    int err = 0;
    struct test_params param = {0};
    float avg;
    unsigned long t_us;

    printf("Creating 2 Tasks \r\n");

    struct timeval tv_start, tv_end;
    param.addr = malloc(1024);

    if (gettimeofday(&tv_start, NULL) == -1) {
        printf("[Error] gettimeofday start failed \r\n");
        return -1;
    }

    err = pthread_create(&thread1, NULL, task1, &param);
    if (err != 0) {
        printf("[Error] can't create thread1");
        return err;
    }

    err = pthread_create(&thread2, NULL, task2, &param);
    if (err != 0) {
        printf("[Error] can't create thread2");
        return err;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    if (gettimeofday(&tv_end, NULL) == -1) {
        printf("[Error] gettimeofday end failed \r\n");
        return -1;
    }
    t_us = tv_end.tv_sec * 1000000 + tv_end.tv_usec - (tv_start.tv_sec * 1000000 + tv_start.tv_usec);
    avg = ((float)t_us)/1000000/4;
    printf("[Total Consume] %ld us \r\n", t_us);
    printf("[Average Latency] %f us \r\n", avg);
    printf("Main Thread left \r\n");

}
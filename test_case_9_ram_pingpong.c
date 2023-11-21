#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define DEBUG 0


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
    int* val1_ref = bar;
    int* val2_ref = val1_ref + 1;
    int val1;
    int val2;

    int g_count = 0;
    while(1)
	{
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 1 do init \r\n");
            g_init = 1;
            val1 = *val1_ref = 0;
            val2 = 0;
            *val2_ref = 1;
        } else {
            val2 = *val2_ref -1;
        }
        val1 = *val1_ref = val1 + 1;
#if DEBUG
        printf("[Info] Thread 1 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
#endif        
        while(*val2_ref != val2 + 1);
        val2 = val2 + 1;
		pthread_cond_signal(&cond2);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond1,&mutex);
		pthread_mutex_unlock(&mutex);
	}
    printf("[DEBUG] Task 1 left \r\n");
    return NULL;
}


void* task2(void* arg)
{
    struct test_params * p_param = (struct test_params *)arg;
    void* bar = p_param->addr;
    int* val1_ref = bar;
    int* val2_ref = val1_ref + 1;
    int val2;
    int val1;
    
    int g_count = 0;

    while(1){
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 2 do init \r\n");
            g_init = 1;
            val2 = *val2_ref = 0;
            val1 = 0;
            *val1_ref = 1;
        } else {
            val1 = *val1_ref -1;
        }
		val2 = *val2_ref = val2 + 1;
		//printf("[Info] Thread 2 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
        while(*val1_ref != val1 + 1);
        val1 = val1 + 1;
		pthread_cond_signal(&cond1);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond2,&mutex);
		pthread_mutex_unlock(&mutex);
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
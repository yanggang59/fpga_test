#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int g_running_cycles = 0;
void* g_mem = NULL;
volatile int g_init = 0;

#define RUNNING_CYCLE_LIMITS     1000000

void* task1(void* arg)
{
    int* val1_ref = g_mem;
    int* val2_ref = val1_ref + 1;
    volatile int val1;
    volatile int val2;

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
		//printf("[Info] Thread 1 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
        while(*val2_ref != val2 + 1);
        val2 = *val2_ref;
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
    int* val1_ref = g_mem;
    int* val2_ref = val1_ref + 1;
    volatile int val2;
    volatile int val1;
    
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
        val1 = *val1_ref;
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
    g_mem = malloc(1024);
    memset(g_mem, 0, 1024);

    printf("Creating Tasks \r\n");

    err = pthread_create(&thread1, NULL, task1, NULL);
    if (err != 0) {
        printf("[Error] can't create thread1");
        return err;
    }

    err = pthread_create(&thread2, NULL, task2, NULL);
    if (err != 0) {
        printf("[Error] can't create thread2");
        return err;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    free(g_mem);

    printf("Main Thread Leaves \r\n");

}
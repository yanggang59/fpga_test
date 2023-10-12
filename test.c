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

void* task1(void* arg)
{
    int* val1_ref = g_mem;
    int* val2_ref = val1_ref + 1;
    volatile int val1 = *val1_ref;// = 1;
    volatile int val2 = 0;
    *val2_ref = 1;
    while(1)
	{
		pthread_mutex_lock(&mutex);
        //sleep(1);
        /**
        * inc area 1
        */
        val1 = *val1_ref = val1 + 1;
		printf("[Info] Thread 1 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
        while(*val2_ref != val2 + 1);
        val2 = *val2_ref;
		pthread_cond_signal(&cond2);
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
    volatile int val2 = *val2_ref = 1;
    volatile int val1 = 0;
    usleep(1);
    while(1){
		pthread_mutex_lock(&mutex);
        //sleep(1);
		val2 = *val2_ref = val2 + 1;
		printf("[Info] Thread 2 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
        while(*val1_ref != val1 + 1);
        val1 = *val1_ref;
		pthread_cond_signal(&cond1);
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
        printf("[Error] can't create producer thread");
        return err;
    }

    err = pthread_create(&thread2, NULL, task2, NULL);
    if (err != 0) {
        printf("[Error] can't create consumer thread");
        return err;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    free(g_mem);

    printf("Main Thread Leaves \r\n");

}
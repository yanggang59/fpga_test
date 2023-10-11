#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_ready = PTHREAD_COND_INITIALIZER;

int g_running_cycles = 0;
void* g_mem = NULL;

void* producer_task(void* arg)
{
    printf("[DEBUG] producer In \r\n");
    void* bar0 = g_mem;
    int* producer_ref = bar0;
    int* consumer_ref = producer_ref + 1;
    int producer_val_bak = 0;
    int consumer_val_bak = 0;
    printf("[DEBUG] producer running 1\r\n");
    *producer_ref = 0;
    while(1) {
        printf("[DEBUG] producer running \r\n");
        if(++g_running_cycles > 1000000) break;
        pthread_mutex_lock(&mutex);
        while(*consumer_ref != consumer_val_bak + 1);
        consumer_val_bak = *consumer_ref;
        producer_val_bak = *producer_ref;
        *producer_ref = producer_val_bak + 1;
        pthread_mutex_unlock(&mutex);
        //pthread_cond_signal(&producer_ready);
    }
    printf("[DEBUG] producer left \r\n");
}


void* consumer_task(void* arg)
{
    printf("[DEBUG] consumer In \r\n");
    void* bar2 = g_mem;
    int* producer_ref = bar2;
    int* consumer_ref = producer_ref + 1;
    int producer_val_bak = 0;
    int consumer_val_bak = 0;
    printf("[DEBUG] consumer running 1\r\n");
    /**
    * Need to set consumer as 1, so producer can start first
    */
    *consumer_ref = 1;
    while(1) {
        printf("[DEBUG] consumer running \r\n");
        if(g_running_cycles > 1000) break;
        pthread_mutex_lock(&mutex);
        while(*producer_ref != producer_val_bak + 1);
        producer_val_bak = *producer_ref;
        //pthread_cond_wait(&producer_ready, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    printf("[DEBUG] consumer left \r\n");
}

int main()
{
    pthread_t thread_producer, thread_consumer;
    int err = 0;
    g_mem = malloc(1024);

    printf("Creating Producer Task \r\n");

    err = pthread_create(&thread_producer, NULL, producer_task, NULL);
    if (err != 0) {
        printf("[Error] can't create producer thread");
        return err;
    }

    printf("Creating Consumer Task \r\n");
    err = pthread_create(&thread_consumer, NULL, consumer_task, NULL);
    if (err != 0)
        printf("[Error] can't create consumer thread");
        return err;

    printf("Deleting Tasks \r\n");

    pthread_join(thread_producer, NULL);
    pthread_join(thread_consumer, NULL);

    free(g_mem);

    printf("Main Thread Leaves \r\n");

}
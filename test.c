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
pthread_cond_t consumer_ready = PTHREAD_COND_INITIALIZER;

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
    *producer_ref = 0;
    while(1) {
        printf("[DEBUG] producer running \r\n");
        if(++g_running_cycles > 1000) break;
        printf("[DEBUG] producer tring to lock \r\n");
        pthread_mutex_lock(&mutex);
        printf("[DEBUG] producer lock , consumer = %d, consumer_bak = %d\r\n", *consumer_ref, consumer_val_bak);
        while(*consumer_ref != consumer_val_bak + 1);
        consumer_val_bak = *consumer_ref;
        producer_val_bak = *producer_ref;
        *producer_ref = producer_val_bak + 1;
        printf("[DEBUG] producer unlock \r\n");
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&producer_ready);
        pthread_cond_wait(&consumer_ready, &mutex);
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
    /**
    * Need to set consumer as 1, so producer can start first
    */
    *consumer_ref = 1;
    while(1) {
        printf("[DEBUG] consumer running \r\n");
        if(g_running_cycles > 1000) break;
        printf("[DEBUG] consumer wait for condition , consumer = %d, consumer_bak = %d\r\n", *consumer_ref, consumer_val_bak);
        pthread_cond_wait(&producer_ready, &mutex);
        printf("[DEBUG] consumer tring to lock \r\n");
        pthread_mutex_lock(&mutex);
        printf("[DEBUG] consumer lock , producer = %d, producer_bak = %d\r\n", *consumer_ref, consumer_val_bak);
        while(*producer_ref != producer_val_bak + 1);
        producer_val_bak = *producer_ref;
        consumer_val_bak = *consumer_ref;
        *consumer_ref = consumer_val_bak + 1;
        printf("[DEBUG] consumer unlock \r\n");
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&consumer_ready);
    }
    printf("[DEBUG] consumer left \r\n");
}

int main()
{
    pthread_t thread_producer, thread_consumer;
    int err = 0;
    g_mem = malloc(1024);

    printf("Creating Tasks \r\n");

    err = pthread_create(&thread_producer, NULL, producer_task, NULL);
    if (err != 0) {
        printf("[Error] can't create producer thread");
        return err;
    }

    err = pthread_create(&thread_consumer, NULL, consumer_task, NULL);
    if (err != 0) {
        printf("[Error] can't create consumer thread");
        return err;
    }

    pthread_join(thread_producer, NULL);
    pthread_join(thread_consumer, NULL);

    free(g_mem);

    printf("Main Thread Leaves \r\n");

}
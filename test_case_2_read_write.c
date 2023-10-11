#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define UIO_DEV "/dev/uio0"

#define UIO_ADDR0 "/sys/class/uio/uio0/maps/map0/addr"
#define UIO_SIZE0 "/sys/class/uio/uio0/maps/map0/size"

#define UIO_ADDR1 "/sys/class/uio/uio0/maps/map1/addr"
#define UIO_SIZE1 "/sys/class/uio/uio0/maps/map1/size"

#define UIO_ADDR2 "/sys/class/uio/uio0/maps/map2/addr"
#define UIO_SIZE2 "/sys/class/uio/uio0/maps/map2/size"


struct map_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
    void* addr2;
    size_t size2;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_ready = PTHREAD_COND_INITIALIZER;
int g_running_cycles = 0;

void print_buf(char* buf, int len)
{
  printf("**************************************************************************************\r\n");
  printf("     ");
  for(int i = 0; i < 16; i++) 
    printf("%4X ", i);

  for(int j = 0; j < len; j++) {
    if(j % 16 == 0) {
      printf("\n%4X ", j);
    }
    printf("%4X ", buf[j]);
  }

  printf("\n**************************************************************************************\r\n");
}

int read_uio_configs(struct map_params* params)
{
    int uio_fd, addr_fd, size_fd;
    int uio_size;
    void* uio_addr, *access_address;
    char uio_addr_buf[64], uio_size_buf[64];

    uio_fd = open(UIO_DEV, O_RDWR);

    addr_fd = open(UIO_ADDR0, O_RDONLY);
    size_fd = open(UIO_SIZE0, O_RDONLY);
    if( addr_fd < 0 || size_fd < 0 || uio_fd < 0) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }
    read(addr_fd, uio_addr_buf, sizeof(uio_addr_buf));
    read(size_fd, uio_size_buf, sizeof(uio_size_buf));

    close(addr_fd);
    close(size_fd);

    uio_addr = (void *)strtoul(uio_addr_buf, NULL, 0);
    uio_size = (int)strtol(uio_size_buf, NULL, 0);

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, 0);

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }

    params->addr0 = access_address;
    params->size0 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(UIO_ADDR1, O_RDONLY);
    size_fd = open(UIO_SIZE1, O_RDONLY);
    if( addr_fd < 0 || size_fd < 0 || uio_fd < 0) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }
    read(addr_fd, uio_addr_buf, sizeof(uio_addr_buf));
    read(size_fd, uio_size_buf, sizeof(uio_size_buf));

    close(addr_fd);
    close(size_fd);

    uio_addr = (void *)strtoul(uio_addr_buf, NULL, 0);
    uio_size = (int)strtol(uio_size_buf, NULL, 0);

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, getpagesize());

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }

    params->addr1 = access_address;
    params->size1 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(UIO_ADDR2, O_RDONLY);
    size_fd = open(UIO_SIZE2, O_RDONLY);
    if( addr_fd < 0 || size_fd < 0 || uio_fd < 0) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }
    read(addr_fd, uio_addr_buf, sizeof(uio_addr_buf));
    read(size_fd, uio_size_buf, sizeof(uio_size_buf));

    close(addr_fd);
    close(size_fd);

    uio_addr = (void *)strtoul(uio_addr_buf, NULL, 0);
    uio_size = (int)strtol(uio_size_buf, NULL, 0);

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, getpagesize() * 2);

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }

    params->addr2 = access_address;
    params->size2 = uio_size;

    close(uio_fd);

    return 0;
}

void* producer_task(void* arg)
{
    struct map_params* params = arg;
    void* bar0 = params->addr0;
    int* producer_ref = bar0;
    int* consumer_ref = producer_ref + 1;
    int producer_val_bak = 0;
    int consumer_val_bak = 0;
    *producer_ref = 0;
    while(1) {
        if(++g_running_cycles > 1000000) break;
        pthread_mutex_lock(&mutex);
        while(*consumer_ref != consumer_val_bak + 1);
        consumer_val_bak = *consumer_ref;
        producer_val_bak = *producer_ref;
        *producer_ref = producer_val_bak + 1;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&producer_ready);
    }
    printf("[DEBUG] producer left \r\n");
}


void* consumer_task(void* arg)
{
    struct map_params* params = arg;
    void* bar2 = params->addr2;
    int* producer_ref = bar2;
    int* consumer_ref = producer_ref + 1;
    int producer_val_bak = 0;
    int consumer_val_bak = 0;
    /**
    * Need to set consumer as 1, so producer can start first
    */
    *consumer_ref = 1;
    while(1) {
        if(g_running_cycles > 1000000) break;
        pthread_mutex_lock(&mutex);
        while(*producer_ref == producer_val_bak);
        producer_val_bak = *producer_ref;
        pthread_cond_wait(&producer_ready, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    printf("[DEBUG] consumer left \r\n");
}

int main()
{
    pthread_t thread_producer, thread_consumer;
    struct map_params params = {0};
    int err = 0;

    if(read_uio_configs(&params)) {
        printf("[Error] read params error");
        return -1;
    }

    err = pthread_create(&thread_producer, NULL, producer_task, &params);
    if (err != 0) {
        printf("[Error] can't create producer thread");
        return err;
    }

    err = pthread_create(&thread_consumer, NULL, consumer_task, &params);
    if (err != 0)
        printf("[Error] can't create consumer thread");
        return err;

    pthread_join(thread_producer, NULL);
    pthread_join(thread_consumer, NULL);

}
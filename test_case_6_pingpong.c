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

struct input_params {
    char* uio_dev;
    char* uio_addr0;
    char* uio_size0;
    char* uio_addr1;
    char* uio_size1;
    char* uio_addr2;
    char* uio_size2;
};

struct output_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
    void* addr2;
    size_t size2;
    int uio_fd;
};



#define RUNNING_CYCLE_LIMITS     1000000

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int g_running_cycles = 0;
volatile int g_init = 0;

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

int read_uio_configs(struct input_params* iparams, struct output_params* oparams)
{
    int uio_fd, addr_fd, size_fd;
    int uio_size;
    void* uio_addr, *access_address;
    char uio_addr_buf[64], uio_size_buf[64];

    uio_fd = open(iparams->uio_dev, O_RDWR);

    addr_fd = open(iparams->uio_addr0, O_RDONLY);
    size_fd = open(iparams->uio_size0, O_RDONLY);
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

    oparams->addr0 = access_address;
    oparams->size0 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(iparams->uio_addr1, O_RDONLY);
    size_fd = open(iparams->uio_size1, O_RDONLY);
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

    oparams->addr1 = access_address;
    oparams->size1 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(iparams->uio_addr2, O_RDONLY);
    size_fd = open(iparams->uio_size2, O_RDONLY);
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

    oparams->addr2 = access_address;
    oparams->size2 = uio_size;

    oparams->uio_fd = uio_fd;

    return 0;
}

void* task0(void* arg)
{
    struct output_params* params = arg;
    void* bar = params->addr1;
    int* val0_ref = bar;
    int* val1_ref = val0_ref + 1;
    int val0;
    int val1;

    int g_count = 0;
    while(1)
	{
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 0 do init \r\n");
            g_init = 1;
            val0 = *val0_ref = 0;
            val1 = 0;
            *val1_ref = 1;
        } else {
            val1 = *val1_ref - 1;
        }
        val0 = *val0_ref = val0 + 1;
#if DEBUG
        printf("[Info] Thread 0 , val0_ref = %d, val0 = %d, val1_ref = %d, val1 = %d \n", *val0_ref, val0, *val1_ref, val1);
#endif        
        while(*val1_ref != val1 + 1);
        val1 = val1 + 1;
		pthread_cond_signal(&cond2);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond1,&mutex);
		pthread_mutex_unlock(&mutex);
	}
    printf("[DEBUG] Task 0 left \r\n");
    return NULL;
}


void* task1(void* arg)
{
    struct output_params* params = arg;
    void* bar = params->addr1;
    int* val0_ref = bar;
    int* val1_ref = val0_ref + 1;
    int val1;
    int val0;
    
    int g_count = 0;

    while(1){
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 1 do init \r\n");
            g_init = 1;
            val1 = *val1_ref = 0;
            val0 = 0;
            *val0_ref = 1;
        } else {
            val0 = *val0_ref - 1;
        }
		val1 = *val1_ref = val1 + 1;
#if DEBUG
		printf("[Info] Thread 1 , val0_ref = %d, val0 = %d, val1_ref = %d, val1 = %d \n", *val0_ref, val0, *val1_ref, val1);
#endif
        while(*val0_ref != val0 + 1);
        val0 = val0 + 1;
		pthread_cond_signal(&cond1);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond2,&mutex);
		pthread_mutex_unlock(&mutex);
	}
    printf("[DEBUG] Task 1 left \r\n");
	return NULL;
}

int main()
{
    pthread_t thread0, thread1;
    int err = 0;
    struct output_params oparams0 = {0};
    struct output_params oparams1 = {0};
    float avg;
    unsigned long t_us;

    struct input_params iparams1 = {
        .uio_dev = "/dev/uio0",
        .uio_addr0 = "/sys/class/uio/uio0/maps/map0/addr",
        .uio_size0 = "/sys/class/uio/uio0/maps/map0/size",
        .uio_addr1 = "/sys/class/uio/uio0/maps/map1/addr",
        .uio_size1 = "/sys/class/uio/uio0/maps/map1/size",
        .uio_addr2 = "/sys/class/uio/uio0/maps/map2/addr",
        .uio_size2 = "/sys/class/uio/uio0/maps/map2/size",
    };
    struct input_params iparams2 = {
        .uio_dev = "/dev/uio1",
        .uio_addr0 = "/sys/class/uio/uio1/maps/map0/addr",
        .uio_size0 = "/sys/class/uio/uio1/maps/map0/size",
        .uio_addr1 = "/sys/class/uio/uio1/maps/map1/addr",
        .uio_size1 = "/sys/class/uio/uio1/maps/map1/size",
        .uio_addr2 = "/sys/class/uio/uio1/maps/map2/addr",
        .uio_size2 = "/sys/class/uio/uio1/maps/map2/size",
    };

    if(read_uio_configs(&iparams1, &oparams0)) {
        printf("[Error] read params 1 error");
        return -1;
    }

    if(read_uio_configs(&iparams2, &oparams1)) {
        printf("[Error] read params 2 error");
        return -1;
    }

    printf("Creating 2 Tasks \r\n");
    struct timeval tv_start, tv_end;

    if (gettimeofday(&tv_start, NULL) == -1) {
        printf("[Error] gettimeofday start failed \r\n");
        return -1;
    }

    err = pthread_create(&thread0, NULL, task0, &oparams0);
    if (err != 0) {
        printf("[Error] can't create thread0");
        return err;
    }

    err = pthread_create(&thread1, NULL, task1, &oparams1);
    if (err != 0) {
        printf("[Error] can't create thread1");
        return err;
    }

    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    if (gettimeofday(&tv_end, NULL) == -1) {
        printf("[Error] gettimeofday end failed \r\n");
        return -1;
    }
    t_us = tv_end.tv_sec * 1000000 + tv_end.tv_usec - (tv_start.tv_sec * 1000000 + tv_start.tv_usec);
    avg = ((float)t_us)/1000000/4;
    printf("[Total Consume] %ld us \r\n", t_us);
    printf("[Average Latency] %f us \r\n", avg);
    printf("Main Thread left \r\n");

    munmap(oparams0.addr0, oparams0.size0);
    munmap(oparams0.addr1, oparams0.size1);
    munmap(oparams0.addr2, oparams0.size2);

    munmap(oparams1.addr0, oparams1.size0);
    munmap(oparams1.addr1, oparams1.size1);
    munmap(oparams1.addr2, oparams1.size2);

    close(oparams0.uio_fd);
    close(oparams1.uio_fd);
}
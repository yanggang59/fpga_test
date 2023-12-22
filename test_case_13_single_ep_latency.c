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

#define RUNNING_CYCLE_LIMITS                          1000000

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

    close(uio_fd);

    return 0;
}

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
        ** This is the code for Thread1, Thread1 starts first
        */
        // Thread1 should stop here until Thread2 runs
        // increase val0
        val0 = *val0_ref = val0 + 1;
#if DEBUG_THIS_MODULE
        printf("[Before Thread1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        while(*val1_ref != val1 + 1);
        val1 = val1 + 1;
#if DEBUG_THIS_MODULE
        printf("[After Thread1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
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
        ** This is the code for Thread2, Thread2 starts later
        */
#if DEBUG_THIS_MODULE
        printf("[Before Thread2] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        // Thread2 should stop here until Thread1 runs
        while(*val0_ref != val0 + 1);
        // increase val1 first, so Thread1 can continue to run
        val1 = *val1_ref = val1 + 1;
        val0 = val0 + 1;
#if DEBUG_THIS_MODULE
        printf("[Thread2] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
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

    struct output_params oparams0 = {0};

    struct input_params iparams0 = {
        .uio_dev = "/dev/uio0",
        .uio_addr0 = "/sys/class/uio/uio0/maps/map0/addr",
        .uio_size0 = "/sys/class/uio/uio0/maps/map0/size",
        .uio_addr1 = "/sys/class/uio/uio0/maps/map1/addr",
        .uio_size1 = "/sys/class/uio/uio0/maps/map1/size",
        .uio_addr2 = "/sys/class/uio/uio0/maps/map2/addr",
        .uio_size2 = "/sys/class/uio/uio0/maps/map2/size",
    };

    if(read_uio_configs(&iparams0, &oparams0)) {
        printf("[Error] read params 1 error");
        return -1;
    }

    printf("Creating 2 Tasks \r\n");

    struct timeval tv_start, tv_end;
    param.addr = (char*)oparams0.addr1 + 0x100;

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
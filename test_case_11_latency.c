#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#define UIO "uio0"

#define UIO_DEV "/dev/"UIO

#define UIO_ADDR0 "/sys/class/uio/"UIO"/maps/map0/addr"
#define UIO_SIZE0 "/sys/class/uio/"UIO"/maps/map0/size"

#define UIO_ADDR1 "/sys/class/uio/"UIO"/maps/map1/addr"
#define UIO_SIZE1 "/sys/class/uio/"UIO"/maps/map1/size"

#define RUNNING_CYCLE_LIMITS                          1000000
#define DEBUG_THIS_MODULE                             0

static char uio_addr_buf[16], uio_size_buf[20];

struct map_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
};

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
#if 0
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
#endif
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

    close(uio_fd);

    return 0;
}


int main()
{
    int count = 0;
    int* bar = NULL;
    int* val0_ref;
    int* val1_ref;
    int val0 = 0, val1 = 0;
    int init = 0;
    struct timeval tv_start, tv_end;
    struct map_params params = {0};
    unsigned long t_us;
    float avg;
    if(read_uio_configs(&params)) {
        printf("[Error] read params error");
        return -1;
    }
    bar = params.addr1;
#ifdef EP0
    memset(params.addr1, 0, 1024);
#endif
    val0_ref = bar;
    val1_ref = bar + 1;
#if 1
    if (gettimeofday(&tv_start, NULL) == -1) {
        printf("[Error] gettimeofday start failed \r\n");
        return -1;
    }
    while(1) {
#ifdef EP0
        /**
        ** This is the code for EP0, EP0 starts first
        */
        //EP0 should stop here until EP1 runs
        //increase val0
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
#endif
#ifdef EP1
        /**
        ** This is the code for EP1, EP1 starts later
        */
#if DEBUG_THIS_MODULE
        printf("[Before EP1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        //EP1 should stop here until EP0 runs
        while(*val0_ref != val0 + 1);
        //increase val1 first, so EP0 can continue to run
        val1 = *val1_ref = val1 + 1;
        val0 = val0 + 1;
#if DEBUG_THIS_MODULE
        printf("[EP1] val0 = %d , val1 = %d , count = %d \r\n", val0, val1, count);
#endif
        if(++count >= RUNNING_CYCLE_LIMITS) {
            break;
        }
#endif
    }
    if (gettimeofday(&tv_end, NULL) == -1) {
        printf("[Error] gettimeofday start failed \r\n");
        return -1;
    }
    t_us = tv_end.tv_sec * 1000000 + tv_end.tv_usec - (tv_start.tv_sec * 1000000 + tv_start.tv_usec);
    avg = ((float)t_us)/1000000/4;
    printf("[Total Consume] %ld us \r\n", t_us);
    printf("[Average Latency] %f us \r\n", avg);
    printf("Main Thread left \r\n");
#endif
    return 0;
}
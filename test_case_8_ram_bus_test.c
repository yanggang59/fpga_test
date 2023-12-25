#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

enum test_mode {
  TEST_DDR = 0,
  TEST_HBM = 1,
};

#define UIO "uio1"

#define UIO_DEV "/dev/"UIO

#define UIO_ADDR0 "/sys/class/uio/"UIO"/maps/map0/addr"
#define UIO_SIZE0 "/sys/class/uio/"UIO"/maps/map0/size"

#define UIO_ADDR1 "/sys/class/uio/"UIO"/maps/map1/addr"
#define UIO_SIZE1 "/sys/class/uio/"UIO"/maps/map1/size"

#define UIO_ADDR2 "/sys/class/uio/"UIO"/maps/map2/addr"
#define UIO_SIZE2 "/sys/class/uio/"UIO"/maps/map2/size"



static char uio_addr_buf[16], uio_size_buf[20];

struct map_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
    void* addr2;
    size_t size2;
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

    printf("=====================================================\r\n");

    close(uio_fd);

    return 0;
}


int main()
{
    struct map_params params = {0};

    if(read_uio_configs(&params)) {
        printf("[Error] read params error");
        return -1;
    }
    enum test_mode test_mode = TEST_DDR;
    int* bar1 = params.addr1;
    *(bar1) = test_mode;
    int* test_space = (int*)((char*)bar1 + 0x100);
    printf("**** Test DDR ****\r\n");
    for(int i = 1; i <= 1000; i++) {
      *test_space = i;
      if( *test_space != i) {
        printf("[Error] *test_space = %d, expect %d\r\n", *test_space, i);
        break;
      }
      printf("test_space = %d \r\n", *test_space);
    }
    test_mode = TEST_HBM;
    *bar1 = test_mode;
    printf("**** Test HBM ****\r\n");
    for(int i = 1; i <= 1000; i++) {
      *test_space = i;
      if( *test_space != i) {
        printf("[Error] *test_space = %d, expect %d\r\n", *test_space, i);
        break;
      }
      printf("test_space = %d \r\n", *test_space);
    }
    printf("The device address %p (lenth %ld)\n", params.addr0, params.size0);
 
    return 0;
}
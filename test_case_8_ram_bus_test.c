#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <time.h>

typedef unsigned long long u64;
#define ULL (unsigned long long)

//DDR [0-16GB]
#define DDR_START           0
#define DDR_SIZE            ULL(16ULL * 1024 * 1024 * 1024)
#define DDR_END             ULL(DDR_START + DDR_SIZE)
#define DDR_TEST_SIZE       ULL(1ULL * 1024 * 1024 * 1024)
#define DDR_TEST_END        ULL(DDR_START + DDR_TEST_SIZE)

//HBM [16GB-20GB]
#define HBM_START           ULL(16ULL * 1024 * 1024 * 1024)
#define HBM_SIZE            ULL(4ULL * 1024 * 1024 * 1024)
#define HBM_END             ULL(HBM_START + HBM_SIZE)
#define HBM_TEST_SIZE       ULL(1ULL * 1024 * 1024 * 1024)
#define HBM_TEST_END        ULL(HBM_START + HBM_TEST_SIZE)

#define TEST_DDR               1
#define TEST_HBM               1
#define DEBUG_THIS_MODULE      1


#define UIO "uio0"

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
    u64 uio_size;
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
        fprintf(stderr, "mmap0: %s\n", strerror(errno));
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
    uio_size = strtol(uio_size_buf, NULL, 0);

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, getpagesize());

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap1: %s\n", strerror(errno));
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
        fprintf(stderr, "mmap2: %s\n", strerror(errno));
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
    u64 offset;
    u64 test_cnt;
    int* test_space;
    int r;

    // set random seed
    srand(time(NULL));
    if(read_uio_configs(&params)) {
        printf("[Error] read params error");
        return -1;
    }
    int* bar1 = params.addr1;
#if TEST_DDR
    test_cnt = 0;
    printf("**** Test DDR ****\r\n");
    for(offset = DDR_START; offset < DDR_TEST_END; offset +=4) {
        test_space = (int*)((char*)bar1 + offset);
        r = rand();
        *test_space = r;
        if( *test_space != r) {
            printf("[Error] Test DDR : offset = %#llx, *test_space = %d, expect %d\r\n", offset, *test_space, r);
            goto error;
        }
#if DEBUG_THIS_MODULE
        if((test_cnt++) % (1024 * 1024) == 0)
            printf("[Info] Test DDR : offset = %#llx, *test_space = %d, expect %d\r\n", offset, *test_space, r);
#endif
    }
#endif

#if TEST_HBM
    printf("**** Test HBM ****\r\n");
    test_cnt = 0;
    for(offset = HBM_START; offset < HBM_TEST_END; offset += 4) {
        test_space = (int*)((char*)bar1 + offset);
        r = rand();
        int read_val;
        *test_space = r;
        read_val = *test_space;
        if( read_val != r) {
            printf("[Error] Test HBM : offset = %#llx, read_val = %d, expect %d\r\n", offset, read_val, r);
            goto error;
        }
#if DEBUG_THIS_MODULE
        if((test_cnt++) % (1024 * 1024) == 0)
            printf("[Info] Test HBM : offset = %#llx, *test_space = %d, expect %d\r\n", offset, *test_space, r);
#endif
    }
#endif
    printf("[Info] IO Test Done \r\n");
    return 0;
error:
    return -1;
}
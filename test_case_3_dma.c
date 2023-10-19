#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#define UIO_DEV "/dev/uio0"

#define UIO_ADDR0 "/sys/class/uio/uio0/maps/map0/addr"
#define UIO_SIZE0 "/sys/class/uio/uio0/maps/map0/size"

#define UIO_ADDR1 "/sys/class/uio/uio0/maps/map1/addr"
#define UIO_SIZE1 "/sys/class/uio/uio0/maps/map1/size"

typedef unsigned int u32;
typedef unsigned long u64;

static char uio_addr_buf[16], uio_size_buf[20];

struct map_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
};

struct dma_desc{
    u32 src_addr_low;
    u32 src_addr_high;
    u32 dst_addr_low;
    u32 reserved;
    u32 length;
}__attribute__((packed));

static void dma_start(char* bar0)
{
    *(u32*)((char*)bar0 + 0x114) = 5;
}

static void dma_enable(char* bar0)
{
    *(u32*)((char*)bar0 + 0) = 1;
}


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

    printf("The device address %p (lenth %ld)\n", params.addr0, params.size0);
    memset(params.addr0, 'K', 32);
    print_buf(params.addr0, 32);


    printf("sizeof(struct dma_desc) = %ld \r\n", sizeof(struct dma_desc));
    dma_enable((char*)params.addr0);

    // struct dma_desc* desc = (struct dma_desc*)((char*)params.addr0 + 0x100);

    // desc->src_addr_low = 0;
    // desc->src_addr_high = 0;
    // desc->dst_addr_low = 0;
    // desc->length = 1024;
    u64 src_addr = 0x100000000;
    u32* addr = (u32 *)((char*)params.addr0 + 0x100);
    *addr = src_addr & 0xFFFFFFFF;
    *(addr + 1) = (src_addr >> 32) & 0xFFFFFFFF;
    *(addr + 2) = 0;
    *(addr + 4) = 0x400;

    dma_start((char*)params.addr0);
    sleep(1);
    printf("sleep done\r\n");
    addr = (u32 *)((char*)params.addr0 + 0x118);
    printf("value@0x118 = 0x%X \r\n", *addr);


    return 0;
}
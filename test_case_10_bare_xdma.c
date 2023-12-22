/**
* Load fpga_uio.ko first , then test
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>


typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

struct input_params {
    char* uio_dev;
    char* uio_addr0;
    char* uio_size0;
    char* uio_addr1;
    char* uio_size1;
    char* uio_addr2;
    char* uio_size2;
    char* uio_addr3;
    char* uio_size3;
    char* uio_addr4;
    char* uio_size4;
};

struct output_params {
    void* addr0;
    size_t size0;
    void* addr1;
    size_t size1;
    void* addr2;
    size_t size2;
    void* addr3;
    size_t size3;
    void* addr4;
    size_t size4;
};

struct dma_parmas{
    uint32_t dma_addr1;
    uint32_t dma_addr2;
    uint32_t desc1_dma;
    uint32_t desc2_dma;
};
/**################# DMA Engine REGS ####################*/

#define H2C_0_REG_BASE_OFF                  0x0000
#define H2C_1_REG_BASE_OFF                  0x0100
#define C2H_0_REG_BASE_OFF                  0x1000
#define C2H_1_REG_BASE_OFF                  0x1100

#define H2C_SGDMA_0_REG_BASE_OFF            0x4000
#define H2C_SGDMA_1_REG_BASE_OFF            0x4100

#define C2H_SGDMA_0_REG_BASE_OFF            0x5000
#define C2H_SGDMA_1_REG_BASE_OFF            0x5100

#define SGDMA_DESC_LOW_ADDR_OFF             0x80
#define SGDMA_DESC_HI_ADDR_OFF              0x84
#define SGDMA_DESC_ADJACENT_OFF             0x88

#define SGDMA_COMMON_REG_BASE_OFF           0x6000
#define MSIX_REG_BASE_OFF                   0x8000


#define CONTROL_OFF                         0x04
#define STATUS_REG_OFF                      0x40
#define CHA_INT_ENA                         0x90

#define IRQ_BLK_REG_BASE_OFF                0x2000
#define CHA_INT_ENA_W1S_OFF                 0x14

#define USER_INT_ENA_MASK_OFF               0x04
#define CHA_INT_ENA_MASK_OFF                0x10

#define CFG_BLK_REG_BASE_OFF                0x3000
#define CFG_BLK_MSI_ENABLE_OFF              0x14


/* bits of the SG DMA control register */
#define XDMA_CTRL_RUN_STOP			        (1UL << 0)
#define XDMA_CTRL_IE_DESC_STOPPED		    (1UL << 1)
#define XDMA_CTRL_IE_DESC_COMPLETED		    (1UL << 2)
#define XDMA_CTRL_IE_DESC_ALIGN_MISMATCH	(1UL << 3)
#define XDMA_CTRL_IE_MAGIC_STOPPED		    (1UL << 4)
#define XDMA_CTRL_IE_IDLE_STOPPED		    (1UL << 6)
#define XDMA_CTRL_IE_READ_ERROR			    (0x1FUL << 9)
#define XDMA_CTRL_IE_DESC_ERROR			    (0x1FUL << 19)
#define XDMA_CTRL_NON_INCR_ADDR			    (1UL << 25)
#define XDMA_CTRL_POLL_MODE_WB			    (1UL << 26)
#define XDMA_CTRL_STM_MODE_WB			    (1UL << 27)


#define DEBUG_DEVICE_FILE "/dev/debug"

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

static void write_reg(void *base_addr, int offset, uint32_t val)
{
    *((uint32_t *)((char*)base_addr + offset)) = val;
}

static uint32_t read_reg(void *base_addr, int offset)
{
    uint32_t read_result = *((uint32_t *)(base_addr + offset));
    return read_result;
}

int read_dma_configs(struct dma_parmas* dma_params)
{
    int fd;
    int ret;
    fd = open(DEBUG_DEVICE_FILE, O_RDWR);
    if(fd < 0) {
        printf("[Error] open fail , fd = %d\r\n", fd);
        return -1;
    }
    uint32_t dma_addr1 = 0;
    uint32_t dma_addr2 = 0;
    uint32_t desc1_dma = 0;
    uint32_t desc2_dma = 0;
    ret = read(fd, &dma_addr1, 8);
    if(ret != 8) {
        printf("[Error] read dma_addr1 fail , ret = %d, dma_addr1 = 0x%x \r\n", ret, dma_addr1);
        close(fd);
        return -1;
    }
    dma_addr1 &= 0xFFFFFFFF;
    printf("[User] read dma_addr1 success , ret = %d, dma_addr1 = 0x%x \r\n", ret, dma_addr1);
    ret = read(fd, &dma_addr2, 8);
    if(ret != 8) {
        printf("[Error] read dma_addr2 fail , ret = %d, dma_addr2 = 0x%x \r\n", ret, dma_addr2);
        close(fd);
        return -1;
    }
    dma_addr2 &= 0xFFFFFFFF;
    printf("[User] read dma_addr1 success , ret = %d, dma_addr2 = 0x%x \r\n", ret, dma_addr2);
    ret = read(fd, &desc1_dma, 8);
    if(ret != 8) {
        printf("[Error] read desc1_dma fail , ret = %d, desc1_dma = 0x%x \r\n", ret, desc1_dma);
        close(fd);
        return -1;
    }
    desc1_dma &= 0xFFFFFFFF;
    printf("[User] read desc1_dma success , ret = %d, desc1_dma = 0x%x \r\n", ret, desc1_dma);
    ret = read(fd, &desc2_dma, 8);
    if(ret != 8) {
        printf("[Error] read desc2_dma fail , ret = %d, desc2_dma = 0x%x \r\n", ret, desc2_dma);
        close(fd);
        return -1;
    }
    desc2_dma &= 0xFFFFFFFF;
    printf("[User] read desc2_dma success , ret = %d, desc2_dma = 0x%x \r\n", ret, desc2_dma);
    close(fd);
    dma_params->dma_addr1 = dma_addr1;
    dma_params->dma_addr2 = dma_addr2;
    dma_params->desc1_dma = desc1_dma;
    dma_params->desc2_dma = desc2_dma;


    return 0;
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
        fprintf(stderr, "mmap 0 : %s\n", strerror(errno));
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
        fprintf(stderr, "mmap 1: %s\n", strerror(errno));
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
        fprintf(stderr, "mmap 2: %s\n", strerror(errno));
        exit(-1);
    }

    oparams->addr2 = access_address;
    oparams->size2 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(iparams->uio_addr3, O_RDONLY);
    size_fd = open(iparams->uio_size3, O_RDONLY);
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

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, getpagesize() * 3);

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap 3: %s\n", strerror(errno));
        exit(-1);
    }

    oparams->addr3 = access_address;
    oparams->size3 = uio_size;

    printf("=====================================================\r\n");

    addr_fd = open(iparams->uio_addr4, O_RDONLY);
    size_fd = open(iparams->uio_size4, O_RDONLY);
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

    access_address = mmap(NULL, uio_size, PROT_READ | PROT_WRITE, MAP_SHARED, uio_fd, getpagesize() * 4);

    if (access_address == (void*) -1) {
        fprintf(stderr, "mmap 4: %s\n", strerror(errno));
        exit(-1);
    }

    oparams->addr4 = access_address;
    oparams->size4 = uio_size;

    close(uio_fd);

    return 0;
}

static void config_control(void* base_addr)
{
    write_reg(base_addr, H2C_0_REG_BASE_OFF | CONTROL_OFF, 0b1111);
    write_reg(base_addr, H2C_1_REG_BASE_OFF | CONTROL_OFF, 0b1111);
    write_reg(base_addr, C2H_0_REG_BASE_OFF | CONTROL_OFF, 0b1111);
    write_reg(base_addr, C2H_1_REG_BASE_OFF | CONTROL_OFF, 0b1111);
}

static void enable_interrupt(void* base_addr)
{
    write_reg(base_addr, H2C_0_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, H2C_1_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, C2H_0_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, C2H_1_REG_BASE_OFF | CHA_INT_ENA, 0b100);
}

static void config_irq_block_reg(void* base_addr)
{
    write_reg(base_addr, IRQ_BLK_REG_BASE_OFF | CHA_INT_ENA_W1S_OFF , ~0);
}

uint32_t swap32(uint32_t value)
{
	return ((value & 0x000000FF) << 24) |
			((value & 0x0000FF00) << 8) |
			((value & 0x00FF0000) >> 8) |
			((value & 0xFF000000) >> 24) ;
}

static void start_h2c(void* base_addr, uint32_t desc_dma, unsigned long adjacent_cnt)
{
    write_reg(base_addr, H2C_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_LOW_ADDR_OFF, desc_dma & 0xFFFFFFFF);
    write_reg(base_addr, H2C_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_HI_ADDR_OFF, ((desc_dma >> 16) >> 16) & 0xFFFFFFFF);
    write_reg(base_addr, H2C_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_ADJACENT_OFF, adjacent_cnt);

    uint32_t w = 0;
    w = (uint32_t)XDMA_CTRL_RUN_STOP;
	w |= (uint32_t)XDMA_CTRL_IE_READ_ERROR;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_ERROR;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
	w |= (uint32_t)XDMA_CTRL_IE_MAGIC_STOPPED;
    w |= (uint32_t)XDMA_CTRL_IE_DESC_STOPPED;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_COMPLETED;

    write_reg(base_addr, H2C_0_REG_BASE_OFF | CONTROL_OFF, w);
}

static void start_c2h(void* base_addr, uint32_t desc_dma, unsigned long adjacent_cnt)
{
    write_reg(base_addr, C2H_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_LOW_ADDR_OFF, desc_dma & 0xFFFFFFFF);
    write_reg(base_addr, C2H_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_HI_ADDR_OFF, ((desc_dma >> 16) >> 16) & 0xFFFFFFFF);
    write_reg(base_addr, C2H_SGDMA_0_REG_BASE_OFF + SGDMA_DESC_ADJACENT_OFF, adjacent_cnt);

    uint32_t w = 0;
    w = (uint32_t)XDMA_CTRL_RUN_STOP;
	w |= (uint32_t)XDMA_CTRL_IE_READ_ERROR;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_ERROR;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_ALIGN_MISMATCH;
	w |= (uint32_t)XDMA_CTRL_IE_MAGIC_STOPPED;
    w |= (uint32_t)XDMA_CTRL_IE_DESC_STOPPED;
	w |= (uint32_t)XDMA_CTRL_IE_DESC_COMPLETED;

    write_reg(base_addr, C2H_0_REG_BASE_OFF | CONTROL_OFF, w);
}

int main()
{
    int err = 0;
    struct output_params oparams0 = {0};
    struct dma_parmas dma_params = {0};

    struct input_params iparams0 = {
        .uio_dev = "/dev/uio0",
        .uio_addr0 = "/sys/class/uio/uio0/maps/map0/addr",
        .uio_size0 = "/sys/class/uio/uio0/maps/map0/size",
        .uio_addr1 = "/sys/class/uio/uio0/maps/map1/addr",
        .uio_size1 = "/sys/class/uio/uio0/maps/map1/size",
        .uio_addr2 = "/sys/class/uio/uio0/maps/map2/addr",
        .uio_size2 = "/sys/class/uio/uio0/maps/map2/size",
        .uio_addr3 = "/sys/class/uio/uio0/maps/map3/addr",
        .uio_size3 = "/sys/class/uio/uio0/maps/map3/size",
        .uio_addr4 = "/sys/class/uio/uio0/maps/map4/addr",
        .uio_size4 = "/sys/class/uio/uio0/maps/map4/size",
    };


    if(read_uio_configs(&iparams0, &oparams0)) {
        printf("[Error] read uio params error");
        return -1;
    }

    if(read_dma_configs(&dma_params)) {
        printf("[Error] read dma params error");
        return -1;
    }

    uint32_t cfg_blk_msi_ena0;
    uint32_t ep0_h2c1_status;

    void* xdma0_config_bar = oparams0.addr1;  // BAR1 ---- 64K

    printf("*** Before DMA ***  \r\n");
    printf("**** DMA0 *****\r\n");
    print_buf(oparams0.addr3, 16);
    printf("**** DMA1 *****\r\n");
    print_buf(oparams0.addr4, 16);

    /**
    * To config DMA Engine of XDMA for EP0, In BAR1
    */
    // CONTROL REG: DMA0, H2C0, H2C1, C2H0, C2H1
    //config_control(xdma0_config_bar);

    // IRQ Block
    config_irq_block_reg(xdma0_config_bar);

    // INT REG: DMA0, H2C0, H2C1, C2H0, C2H1
    enable_interrupt(xdma0_config_bar);

    uint32_t identifier = read_reg(xdma0_config_bar, CFG_BLK_REG_BASE_OFF);
    printf("[Info] identifier = %#x \r\n", identifier);

    cfg_blk_msi_ena0 = read_reg(xdma0_config_bar, CFG_BLK_REG_BASE_OFF | CFG_BLK_MSI_ENABLE_OFF);
    printf("[Info] cfg_blk_msi_ena0 = %d\r\n", cfg_blk_msi_ena0);

    ep0_h2c1_status = read_reg(xdma0_config_bar, H2C_1_REG_BASE_OFF | STATUS_REG_OFF);
    printf("Before DMA , ep0_h2c1_status = %d \r\n", ep0_h2c1_status);

    /**
    * DMA from Host to device
    */
    start_h2c(xdma0_config_bar, dma_params.desc1_dma, 0);

    sleep(1);

    /**
    * DMA from device to Host
    */
    start_c2h(xdma0_config_bar, dma_params.desc2_dma, 0);

    printf("*** After DMA ***  \r\n");
    printf("**** DMA0 *****\r\n");
    print_buf(oparams0.addr3, 16);
    printf("**** DMA1 *****\r\n");
    print_buf(oparams0.addr4, 16);

    ep0_h2c1_status = read_reg(xdma0_config_bar, H2C_1_REG_BASE_OFF | STATUS_REG_OFF);
    printf("After DMA , ep0_h2c1_status = %d \r\n", ep0_h2c1_status);

    munmap(oparams0.addr0, oparams0.size0);
    munmap(oparams0.addr1, oparams0.size1);
    munmap(oparams0.addr2, oparams0.size2);
    munmap(oparams0.addr3, oparams0.size3);
    munmap(oparams0.addr4, oparams0.size4);

}
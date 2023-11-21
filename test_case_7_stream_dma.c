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
};

typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

/**################# DMA Engine REGS ####################*/

#define H2C_0_REG_BASE_OFF                 0x0000
#define H2C_1_REG_BASE_OFF                 0x0100
#define C2H_0_REG_BASE_OFF                 0x1000
#define C2H_1_REG_BASE_OFF                 0x1100

#define CONTROL_OFF                        0x04
#define STATUS_REG_OFF                     0x40
#define CHA_INT_ENA                        0x90

#define IRQ_BLK_REG_BASE_OFF               0x2000

#define USER_INT_ENA_MASK_OFF              0x04
#define CHA_INT_ENA_MASK_OFF               0x10


#define CFG_BLK_REG_BASE_OFF                0x3000
#define CFG_BLK_MSI_ENABLE_OFF              0x14

/**################# USR REGS ####################*/
#define HOST_BASE_ADDR_LO_OFF              0x00
#define HOST_BASE_ADDR_HI_OFF              0x04
#define SRC_PORT_ID_OFF                    0x08
#define SRC_ADDR_OFF                       0x0C
#define DST_PORT_ID_OFF                    0x10
#define DST_ADDR_OFF                       0x14
#define DMA_LENGTH_OFF                     0x18




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
    *((uint32_t *)(base_addr + offset)) = val;
}

static uint32_t read_reg(void *base_addr, int offset)
{
    uint32_t read_result = *((uint32_t *)(base_addr + offset));
    return read_result;
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
        fprintf(stderr, "mmap: %s\n", strerror(errno));
        exit(-1);
    }

    oparams->addr3 = access_address;
    oparams->size3 = uio_size;

    close(uio_fd);

    return 0;
}

static void config_control(void* base_addr)
{
    write_reg(base_addr, H2C_0_REG_BASE_OFF | CONTROL_OFF, 0b101);
    write_reg(base_addr, H2C_1_REG_BASE_OFF | CONTROL_OFF, 0b101);
    write_reg(base_addr, C2H_0_REG_BASE_OFF | CONTROL_OFF, 0b101);
    write_reg(base_addr, C2H_1_REG_BASE_OFF | CONTROL_OFF, 0b101);
}

static void enable_interrupt(void* base_addr)
{
    write_reg(base_addr, H2C_0_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, H2C_1_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, C2H_0_REG_BASE_OFF | CHA_INT_ENA, 0b100);
    write_reg(base_addr, C2H_1_REG_BASE_OFF | CHA_INT_ENA, 0b100);
}

static void config_irq_block(void* xdma_reg_base_addr)
{
    write_reg(xdma_reg_base_addr, IRQ_BLK_REG_BASE_OFF | CHA_INT_ENA_MASK_OFF, 0b1111);
}

static void config_usr_src_regs(void* usr_base_reg_addr, void* src_addr, int src_port_id, int dst_port_id, int src_off, int dst_off)
{
    write_reg(usr_base_reg_addr, HOST_BASE_ADDR_LO_OFF, (unsigned long)src_addr & 0xFFFFFFFF);
    write_reg(usr_base_reg_addr, HOST_BASE_ADDR_HI_OFF, 0);

    write_reg(usr_base_reg_addr, SRC_PORT_ID_OFF, src_port_id);
    write_reg(usr_base_reg_addr, SRC_ADDR_OFF, src_off);

    write_reg(usr_base_reg_addr, DST_PORT_ID_OFF, dst_port_id);
    write_reg(usr_base_reg_addr, DST_ADDR_OFF, dst_off);

}

static void config_usr_dst_regs(void* usr_base_reg_addr, void* dst_addr)
{
    write_reg(usr_base_reg_addr, HOST_BASE_ADDR_LO_OFF, (unsigned long)dst_addr & 0xFFFFFFFF);
    write_reg(usr_base_reg_addr, HOST_BASE_ADDR_HI_OFF, 0);
}

static void start_dma(void* usr_base_reg_addr, int dma_length)
{
    write_reg(usr_base_reg_addr, DMA_LENGTH_OFF, dma_length);
}

int main()
{
    int err = 0;
    struct output_params oparams0 = {0};
    struct output_params oparams1 = {0};

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
    };
    struct input_params iparams1 = {
        .uio_dev = "/dev/uio1",
        .uio_addr0 = "/sys/class/uio/uio1/maps/map0/addr",
        .uio_size0 = "/sys/class/uio/uio1/maps/map0/size",
        .uio_addr1 = "/sys/class/uio/uio1/maps/map1/addr",
        .uio_size1 = "/sys/class/uio/uio1/maps/map1/size",
        .uio_addr2 = "/sys/class/uio/uio1/maps/map2/addr",
        .uio_size2 = "/sys/class/uio/uio1/maps/map2/size",
        .uio_addr3 = "/sys/class/uio/uio1/maps/map3/addr",
        .uio_size3 = "/sys/class/uio/uio1/maps/map3/size",
    };

    if(read_uio_configs(&iparams0, &oparams0)) {
        printf("[Error] read params 1 error");
        return -1;
    }

    if(read_uio_configs(&iparams1, &oparams1)) {
        printf("[Error] read params 2 error");
        return -1;
    }

    void* src_base_addr = (void*)0xfffff000;
    void* dst_base_addr = (void*)(0xffffe000);

    int src_port_id = 0;
    int dst_port_id = 1;

    int src_off = 0;
    int dst_off = 0;

    /**
    * [Note] : write dma_length means start dma
    */
    int dma_length = 4096;

    void* usr0_config_bar = oparams0.addr0;   // BAR0 ---- 1M
    void* xdma0_config_bar = oparams0.addr1;  // BAR1 ---- 64K

    void* usr1_config_bar = oparams1.addr0;   // BAR0 ---- 1M
    void* xdma1_config_bar = oparams1.addr1;  // BAR1 ---- 64K

    printf("*** Before DMA ***  \r\n");
    printf("**** EP0 *****\r\n");
    print_buf(oparams0.addr2, 1024);
    printf("**** EP1 *****\r\n");
    print_buf(oparams1.addr3, 1024);

    /**
    * To config DMA Engine of XDMA for EP0 and EP1, In BAR1
    */
    // CONTROL REG: DMA0, H2C0, H2C1, C2H0, C2H1
    config_control(xdma0_config_bar);

    // CONTROL REG: DMA1, H2C0, H2C1, C2H0, C2H1
    config_control(xdma1_config_bar);

    // INT REG: DMA0, H2C0, H2C1, C2H0, C2H1
    enable_interrupt(xdma0_config_bar);
    // INT REG: DMA1, H2C0, H2C1, C2H0, C2H1
    enable_interrupt(xdma1_config_bar);


    config_irq_block(xdma0_config_bar);
    config_irq_block(xdma1_config_bar);

    uint32_t cfg_blk_msi_ena0 = read_reg(xdma0_config_bar, CFG_BLK_REG_BASE_OFF | CFG_BLK_MSI_ENABLE_OFF);
    uint32_t cfg_blk_msi_ena1 = read_reg(xdma1_config_bar, CFG_BLK_REG_BASE_OFF | CFG_BLK_MSI_ENABLE_OFF);

    printf("[Info] cfg_blk_msi_ena0 = %d, cfg_blk_msi_ena1 = %d \r\n", cfg_blk_msi_ena0, cfg_blk_msi_ena1);

    uint32_t ep0_h2c1_status = read_reg(xdma0_config_bar, H2C_0_REG_BASE_OFF | STATUS_REG_OFF);
    uint32_t ep1_c2h0_status = read_reg(xdma1_config_bar, C2H_0_REG_BASE_OFF | STATUS_REG_OFF);
    printf("Before DMA , ep0_h2c1_status = %d, ep1_c2h0_status = %d \r\n", ep0_h2c1_status, ep1_c2h0_status);
    /**
    * To config USR REG, In BAR0
    */
    config_usr_src_regs(usr0_config_bar, src_base_addr, src_port_id, dst_port_id, src_off, dst_off);
    config_usr_dst_regs(usr1_config_bar, dst_base_addr);


    /**
    * set dma length to start dma 
    */
    start_dma(usr0_config_bar, dma_length);
 
    sleep(1);

    printf("*** After DMA ***  \r\n");
    printf("**** EP0 *****\r\n");
    print_buf(oparams0.addr2, 1024);
    printf("**** EP1 *****\r\n");
    print_buf(oparams1.addr3, 1024);

    ep0_h2c1_status = read_reg(xdma0_config_bar, H2C_0_REG_BASE_OFF | STATUS_REG_OFF);
    ep1_c2h0_status = read_reg(xdma1_config_bar, C2H_0_REG_BASE_OFF | STATUS_REG_OFF);
    printf("Before DMA , ep0_h2c1_status = %d, c2h0_status_1 = %d \r\n", ep0_h2c1_status, ep1_c2h0_status);
}
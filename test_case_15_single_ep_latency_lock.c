#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

typedef unsigned long long u64;
struct test_params {
    void* addr;
};

#define ULL (unsigned long long)
//DDR [0-16GB]
#define DDR_START           0
#define DDR_SIZE            ULL(16ULL * 1024 * 1024 * 1024)
#define DDR_END             ULL(DDR_START + DDR_SIZE)

//HBM [16GB-20GB]
#define HBM_START           ULL(16ULL * 1024 * 1024 * 1024)
#define HBM_SIZE            ULL(4ULL * 1024 * 1024 * 1024)
#define HBM_END             ULL(HBM_START + HBM_SIZE)

#define TEST_DDR            1
#define TEST_HBM            !TEST_DDR
#define TEST_RAM            1

#define DEBUG 0


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
    u64 uio_size;
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
    uio_size = (u64)strtol(uio_size_buf, NULL, 0);

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
    uio_size = (u64)strtol(uio_size_buf, NULL, 0);

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
    uio_size = (u64)strtol(uio_size_buf, NULL, 0);

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
    void* bar0 = p_param->addr;
    int* val1_ref = bar0;
    int* val2_ref = val1_ref + 1;
    int val1;
    int val2;

    int g_count = 0;
    while(1)
	{
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 1 do init \r\n");
            g_init = 1;
            val1 = *val1_ref = 0;
            val2 = 0;
            *val2_ref = 1;
        } else {
            val2 = *val2_ref -1;
        }
        val1 = *val1_ref = val1 + 1;
#if DEBUG
        printf("[Info] Thread 1 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
#endif        
        while(*val2_ref != val2 + 1);
        val2 = val2 + 1;
		pthread_cond_signal(&cond2);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond1,&mutex);
		pthread_mutex_unlock(&mutex);
	}
    printf("[DEBUG] Task 1 left \r\n");
    return NULL;
}


void* task2(void* arg)
{
    struct test_params * p_param = (struct test_params *)arg;
    void* bar2 = p_param->addr;
    int* val1_ref = bar2;
    int* val2_ref = val1_ref + 1;
    int val2;
    int val1;
    
    int g_count = 0;

    while(1){
		pthread_mutex_lock(&mutex);
        if(!g_init) {
            printf("[DEBUG] Task 2 do init \r\n");
            g_init = 1;
            val2 = *val2_ref = 0;
            val1 = 0;
            *val1_ref = 1;
        } else {
            val1 = *val1_ref -1;
        }
		val2 = *val2_ref = val2 + 1;
		//printf("[Info] Thread 2 , val1_ref = %d, val1 = %d, val2_ref = %d, val2 = %d \n", *val1_ref, val1, *val2_ref, val2);
        while(*val1_ref != val1 + 1);
        val1 = val1 + 1;
		pthread_cond_signal(&cond1);
        if(++g_count > RUNNING_CYCLE_LIMITS) {
            pthread_mutex_unlock(&mutex);
            break;
        }
		pthread_cond_wait(&cond2,&mutex);
		pthread_mutex_unlock(&mutex);
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

#if TEST_DDR
    param.addr = (char*)oparams0.addr1 + 0x100;
    printf("[Info] Test DDR , Single EP IO Latency Test\r\n");
#endif
#if TEST_HBM
    param.addr = (char*)oparams0.addr1 + HBM_START + 0x100;
    printf("[Info] Test HBM , Single EP IO Latency Test\r\n");
#endif

    struct timeval tv_start, tv_end;

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
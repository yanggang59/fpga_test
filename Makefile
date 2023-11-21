TESTCASE1 ?= test_case_1_mem_copy.c
TESTCASE2 ?= test_case_2_read_write.c
TESTCASE3 ?= test_case_3_dma.c
TESTCASE4 ?= test_case_4_dma.c
TESTCASE5 ?= test_case_5_pingpong.c
TESTCASE6 ?= test_case_6_pingpong.c
TESTCASE7 ?= test_case_7_stream_dma.c
TESTCASE8 ?= test_case_8_ram_bus_test.c
TESTCASE9 ?= test_case_9_ram_pingpong.c

all:
	gcc ${TESTCASE1} -o test_case_1
	gcc ${TESTCASE2} -o test_case_2 -lpthread
	gcc ${TESTCASE3} -o test_case_3
	gcc ${TESTCASE4} -o test_case_4
	gcc ${TESTCASE5} -o test_case_5 -lpthread
	gcc ${TESTCASE6} -o test_case_6 -lpthread
	gcc ${TESTCASE7} -o test_case_7
	gcc ${TESTCASE8} -o test_case_8
	gcc ${TESTCASE9} -o test_case_9 -lpthread
	gcc test.c -lpthread -g -o test

clean:
	rm test_case_1;
	rm test_case_2
	rm test_case_3
	rm test_case_4
	rm test_case_5
	rm test_case_6
	rm test_case_7
	rm test_case_8
	rm test_case_9
	rm test

TESTCASE1 ?= test_case_1_mem_copy.c
TESTCASE2 ?= test_case_2_read_write.c
TESTCASE3 ?= test_case_3_dma.c
TESTCASE4 ?= test_case_4_dma.c
TESTCASE5 ?= test_case_5_pingpong.c
TESTCASE6 ?= test_case_6_pingpong.c

all:
	gcc ${TESTCASE1} -o test_case_1
	gcc ${TESTCASE2} -o test_case_2 -lpthread
	gcc ${TESTCASE3} -o test_case_3
	gcc ${TESTCASE4} -o test_case_4
	gcc ${TESTCASE5} -o test_case_5 -lpthread -O0
	gcc ${TESTCASE6} -o test_case_6 -lpthread -O0
	gcc test.c -lpthread -O0 -g -o test

clean:
	rm test_case_1
	rm test_case_2
	rm test_case_3
	rm test_case_4
	rm test_case_5
	rm test_case_6
	rm test

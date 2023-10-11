TESTCASE1 ?= test_case_1_mem_copy.c
TESTCASE2 ?= test_case_2_read_write.c

all:
	gcc ${TESTCASE1} -o test_case_1
	gcc ${TESTCASE2} -o test_case_2 -lpthread

clean:
	rm test_case_1
	rm test_case_2
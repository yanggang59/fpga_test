TESTCASE1 ?= test_case_1_mem_copy.c
TESTCASE2 ?= test_case_2_read_write.c
TESTCASE3 ?= test_case_3_dma.c
TESTCASE4 ?= test_case_4_ram_latency_nolock.c
TESTCASE5 ?= test_case_5_pingpong.c
TESTCASE6 ?= test_case_6_pingpong.c
TESTCASE7 ?= test_case_7_stream_dma.c
TESTCASE8 ?= test_case_8_ram_bus_test.c
TESTCASE9 ?= test_case_9_ram_pingpong.c
TESTCASE10 ?= test_case_10_bare_xdma.c
TESTCASE11 ?= test_case_11_latency.c
TESTCASE12 ?= test_case_12_pingpong.c
TESTCASE13 ?= test_case_13_single_ep_latency.c
TESTCASE14 ?= test_case_14_2_ep_latency.c

all:
	gcc ${TESTCASE1} -o test_case_1
	gcc ${TESTCASE2} -o test_case_2 -lpthread
	gcc ${TESTCASE3} -o test_case_3
	gcc ${TESTCASE4} -o test_case_4 -lpthread
	gcc ${TESTCASE5} -o test_case_5 -lpthread
	gcc ${TESTCASE6} -o test_case_6 -lpthread
	gcc ${TESTCASE7} -o test_case_7
	gcc ${TESTCASE8} -o test_case_8
	gcc ${TESTCASE9} -o test_case_9 -lpthread
	gcc ${TESTCASE10} -o test_case_10
	gcc ${TESTCASE11} -o test_case_11_ep0 -DEP0
	gcc ${TESTCASE11} -o test_case_11_ep1 -DEP1
	gcc ${TESTCASE12} -o test_case_12 -lpthread
	gcc ${TESTCASE13} -o test_case_13 -lpthread
	gcc ${TESTCASE14} -o test_case_14 -lpthread

clean:
	rm test_case_1 -f
	rm test_case_2 -f
	rm test_case_3 -f
	rm test_case_4 -f
	rm test_case_5 -f
	rm test_case_6 -f
	rm test_case_7 -f
	rm test_case_8 -f
	rm test_case_9 -f
	rm test_case_10 -f
	rm test_case_11_ep0 -f
	rm test_case_11_ep1 -f
	rm test_case_12 -f
	rm test_case_13 -f
	rm test_case_14 -f

## 1.Load driver

```
sudo ./load_igb.sh VID DID
```

VID is the Vendor ID, DID is the Device ID. Using igb_uio driver, please modify the driver location accordingly.

## 2.Content Explanation

(1) test case 1 is to do memory comparation of BAR0 and BAR2

(2) test case 2 is to test read write latency of PCIe. Create 2 threads to read/write PCIe bar, using mutex and condition varibale to syncronize

(3) test case 3 is to test simple_fpga dma function, we avoid use function call to debug, ugly coding

(4) test case 4 is to test ram access latency no lock version

(5) 
(6) test case 5,6 is to test PCIe bar access latency in 2 EPs

(7) test case 7 is to test stream dma function on Xilinx U280 to transform data from EP0 to EP1, also can test BW and CPU Utilization

(8) test case 8 is ram bus IO test

(9) test case 9 is ram access latency test using mutex and condition variable

(10) test 10 is to test xdma function not using XDMA Official Driver, modified base on test 7 ,not finished yet

(11) test 11 is to test FuDanWei 2 Host accessing its sharing bar EP latency without using lock, will create 2 binary for EP0 and EP1, EP0 runs first 

(12)

(13)

(14)

(15)

(16) Test single EP read/write latency

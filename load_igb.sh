if [ $UID -ne 0 ];
then
 echo "Please run with SU priviledge"
 exit 1;
fi

if [ $# -ne 2 ];
then
    echo "[Info] should input 2 params as Vendor ID and Device ID"
    exit 1
fi
VID=$1
DID=$2
echo "*** Trying to install driver ***"
cd /home/fpga/workspace/dpdk-kmods/linux/igb_uio
modprobe uio
lsmod | grep igb_uio
if [ $? -eq 0 ]; then
	rmmod igb_uio
	if [ $? -ne 0 ]; then
		echo "rmmod igb_uio failed"
		exit 1
	fi
fi
insmod igb_uio.ko
cd /sys/bus/pci/drivers/igb_uio
echo "First argument: $VID"
echo "Second argument: $DID"
echo "$VID $DID"|sudo tee new_id
echo "*** Install driver done ***"
lspci -vd $1556:



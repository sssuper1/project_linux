obj-m := virt-eth0.o
virt-eth0-y += virt_eth0.o
virt-eth0-y += virt_eth_dma.o
virt-eth0-y += virt_eth_frame.o
virt-eth0-y += virt_eth_mgmt.o
virt-eth0-y += virt_eth_queue.o
virt-eth0-y += virt_eth_station.o
virt-eth0-y += virt_eth_system.o
virt-eth0-y += virt_eth_util.o
virt-eth0-y += virt_eth_comm_board.o
virt-eth0-y += virt_eth_jgk.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers

APP = mgmt-ctrl

all:
	$(CC) $(LDFLAGS) -I/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/libnl/usr/include/libnl3 -I/usr/include/libnl3 -I/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/sqlite3/usr/include -g -o $(APP) mgmt_netlink.c mgmt_transmit.c main.c Thread.c socketUDP.c SocketTCP.c Lock.c Pcap.c sqlite_unit.c gpsget.c -L/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/libnl/usr/lib -I/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/libpcap/usr/include -L/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/libpcap/usr/lib -L/home/lu/zynq/zynq_cmb/zynq_mme/build/tmp/sysroots-components/cortexa9hf-neon/sqlite3/usr/lib -lpthread -lnl-genl-3 -lnl-3 -lpcap -lsqlite3
	#$(CC) $(LDFLAGS) -I/opt/pkg/petalinux/zynq_project/build/tmp/sysroots-components/cortexa9hf-neon/libnl/usr/include/libnl3 -g -o $(APP) mgmt_netlink.c mgmt_transmit.c main.c Thread.c socketUDP.c SocketTCP.c Lock.c Pcap.c -L/opt/pkg/petalinux/zynq_project/build/tmp/sysroots-components/cortexa9hf-neon/libnl/usr/lib -I/opt/pkg/petalinux/zynq_project/build/tmp/sysroots-components/cortexa9hf-neon/libpcap/usr/include -L/opt/pkg/petalinux/zynq_project/build/tmp/sysroots-components/cortexa9hf-neon/libpcap/usr/lib -lnl-genl-3 -lnl-3 -lpcap -lnet -lpthreadp

#	$(CC) -g -o $(APP) mgmt_netlink.c mgmt_transmit.c main.c Thread.c socketUDP.c -lpthread -lnl-genl-3 -lnl-3 -lmysqlclient
clean:
	-rm -f *.o mgmt-ctrl0224



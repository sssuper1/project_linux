SUMMARY = "Recipe for  build an external virt-eth0 Linux kernel module"
SECTION = "PETALINUX/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://virt_eth0.c \
           file://virt_eth_dma.c \
           file://virt_eth_frame.c \
           file://virt_eth_mgmt.c \
           file://virt_eth_queue.c \
           file://virt_eth_station.c \
           file://virt_eth_system.c \
           file://virt_eth_util.c \
	   file://virt_eth_comm_board.c \
           file://virt_eth_jgk.c \
           file://virt_eth_util.h \
           file://virt_eth_types.h \
           file://virt_eth_dma.h \
           file://virt_eth_mgmt.h \
           file://virt_eth_station.h \
           file://virt_eth_queue.h \
           file://virt_eth_system.h \
           file://virt_eth_frame.h \
	   file://virt_eth_comm_board.h \
           file://virt_eth_jgk.h \
	   file://COPYING \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

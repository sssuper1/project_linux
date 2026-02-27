#
# This file is the mgmt-ctrl recipe.
#

SUMMARY = "Simple mgmt-ctrl application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
	   file://mgmt_netlink.c \
	   file://mgmt_transmit.c \
	   file://socketUDP.c \
	   file://SocketTCP.c \
	   file://Thread.c \
	   file://Lock.c \
	   file://Pcap.c \
	   file://sqlite_unit.c \
	   file://mgmt_netlink.h \
	   file://mgmt_transmit.h \
	   file://socketUDP.h \
	   file://SocketTCP.h \
	   file://mgmt_types.h \
	   file://Thread.h \
	   file://Lock.h \
	   file://Pcap.h \
           file://sqlite_unit.h \
	   file://Makefile \
           file://gpsget.c \
	   file://gpsget.h \
	   file://wg_config.h\
	file://ui_get.c\
	file://ui_get.h\
	file://enum_uartparam_addr.h\	
	  "

S = "${WORKDIR}"

DEPENDS += "libmysqlclient \
	    libnl \
"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 mgmt-ctrl ${D}${bindir}
}

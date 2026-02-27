SUMMARY = "Recipe for  build an external mgmt-agent Linux kernel module"
SECTION = "PETALINUX/modules"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=12f884d2ae1ff87c09e5b7ccc2c4ca7e"

inherit module

SRC_URI = "file://Makefile \
           file://debugfs.c \
           file://mgmt_module.c \
           file://mgmt_netlink.c \
           file://debugfs.h \
           file://mgmt_module.h \
           file://mgmt_netlink.h \
           file://mgmt_types.h \
	   file://COPYING \
          "

S = "${WORKDIR}"

# The inherit of module.bbclass will automatically name module packages with
# "kernel-module-" prefix as required by the oe-core build environment.

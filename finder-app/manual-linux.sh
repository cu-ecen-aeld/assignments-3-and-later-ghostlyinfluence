#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.
# Edited by: Colin Burgin

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
NPROC=$(nproc --all) # Determine number of Processors
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)


if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # Done: Add your kernel build steps here
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper # Deep clear the kernel build tree
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig # Configure for virt ARM for QEMU
    make -j ${NPROC} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all # Build
    make -j ${NPROC} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules # modules
    make -j ${NPROC} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs # devicetree
fi

echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/Image

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# Done: Create necessary base directories
mkdir "${OUTDIR}/rootfs" # Create root directory
cd "${OUTDIR}/rootfs" # Go here for convience
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

# tree "${OUTDIR}/rootfs"

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # Done:  Configure busybox
    make distclean # Perform the two setup steps from the video
    make defconfig
else
    cd busybox
fi

# Done: Make and install busybox
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} # Perform the two install steps
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

# Next section fails you don't CD here. bin/busybox is not a valid location unless in the rootfs
cd "${OUTDIR}/rootfs"

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

# Done: Add library dependencies to rootfs
echo "Copying dependencies"
# ***From Command Line***
# [Requesting program interpreter: /lib/ld-linux-aarch64.so.1]
# 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
# 0x0000000000000001 (NEEDED)             Shared library: [libresolv.so.2]
# 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
cp ${SYSROOT}/lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib/"
cp ${SYSROOT}/lib64/libm.so.6 "${OUTDIR}/rootfs/lib64/"
cp ${SYSROOT}/lib64/libresolv.so.2 "${OUTDIR}/rootfs/lib64/"
cp ${SYSROOT}/lib64/libc.so.6 "${OUTDIR}/rootfs/lib64/"

# Done: Make device nodes
echo "Making devices"
sudo mknod -m 666 dev/null c 1 3 # Copied from video
sudo mknod -m 666 dev/console c 5 1

# Done: Clean and build the writer utility
echo "Cross compiling finder"
cd "${FINDER_APP_DIR}"
make clean
make CROSS_COMPILE=${CROSS_COMPILE} all

# Done: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "Copying finder files"
cp writer finder.sh finder-test.sh autorun-qemu.sh ${OUTDIR}/rootfs/home/
cp -r conf/ ${OUTDIR}/rootfs/home/

# Done: Chown the root directory
echo "Chowning all the things"
cd "${OUTDIR}/rootfs"
sudo chown -R root:root "${OUTDIR}/rootfs"

# TODO: Create initramfs.cpio.gz
echo "Create the initramfs"
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
gzip -f ${OUTDIR}/initramfs.cpio
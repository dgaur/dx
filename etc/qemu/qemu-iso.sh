#!/bin/sh
#
# Shell script to launch the dx .iso image under qemu emulator
#
# Usage:
#    % qemu-iso.sh [additional qemu flags]
#
# For gdb support:
#	- add '-s' to the qemu command line
#	- start gdb
#	- in gdb console, 'target remote localhost:1234'
#


#
# Launch qemu:
#	- boot from the .iso image
#	- disable PXE boot at startup
#	- emulate a P2 CPU, mostly for luajit
#	- write debug output, if any, to /tmp/serial.txt
#
qemu -cdrom ${DX_ROOT_DIR}/media/iso/dx.iso -net none -cpu pentium2 \
	-serial file:/tmp/serial.txt $*


#!/bin/sh

#
# Launch qemu:
#	- boot from the .iso image
#	- disable PXE boot at startup
#	- emulate a P2 CPU, mostly for luajit
#	- write debug output, if any, to /tmp/serial.txt
#
qemu -cdrom media/iso/dx.iso -net none -cpu pentium2 -serial file:/tmp/serial.txt

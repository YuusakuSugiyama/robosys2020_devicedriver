# SPDX-License-Identifier: GPL-3.0

obj-m := mydriver.o

mydriver.ko: mydriver.c
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 modules

clean:
	make -C /usr/src/linux-headers-`uname -r` M=`pwd` V=1 clean

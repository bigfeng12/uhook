obj-m	+= uhook.o
all:
	make -C /lib/modules/`uname -r`/build M=`pwd` modules
	#build app/uuhook.c
	make -C app
clean:
	make -C /lib/modules/`uname -r`/build M=`pwd` clean

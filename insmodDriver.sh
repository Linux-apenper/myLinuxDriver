
#!/bin/bash

HOME_DIR=./driver/globalfifo

if [ "`cat /proc/devices | grep "globalfifo"`" ]; then
	echo "have install the globalfifo.ko, please firstly remove it."

	# rmmod the mod file
	`sudo rmmod globalfifo`

	# install the mod file
	echo "install the mod file"
	`sudo insmod $HOME_DIR/globalfifo.ko`

	# check the mod file
	echo "################# display the /proc/devices ##################"
	cat /proc/devices | grep "globalfifo*"
fi


if [ -e "/dev/globalfifo" ]; then
	echo "have the globalfifo device name."
	
	# delete the exist file
	`sudo rm /dev/globalfifo`

	# create the device file
	echo "creating the device file: /dev/globalfifo"
	`sudo mknod /dev/globalfifo c 250 0`
fi

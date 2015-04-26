#!/bin/bash

if [ "$(id -u)" != "0" ]; then
	echo "Must be root to run"
	exit 1
fi

mount -o loop disk $PWD/mydisk/
ls -l mydisk
umount mydisk

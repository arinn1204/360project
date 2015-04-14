#!/bin/bash

if [ "$uid" != "0" ]; then
	echo "Must be root to run"
	exit 1
fi

mount -o loop disk mydisk
ls -l mydisk
umount mydisk

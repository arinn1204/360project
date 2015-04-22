#!/bin/bash

if [ "$1" == "normal" ]; then
	gcc -g ../main.c
	./a.out disk

elif [ "$1" == "debug" ]; then
	gcc -g ../main.c
	gdb --args a.out disk
fi

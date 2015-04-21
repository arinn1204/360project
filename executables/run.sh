#!/bin/bash

gcc -g ../main.c
rm disk
touch disk
mkfs.ext2 disk 1440
./a.out disk

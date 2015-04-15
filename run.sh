#!/bin/bash


rm disk
touch disk
mkfs.ext2 disk 1440
./a.out disk

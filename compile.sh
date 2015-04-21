#!/bin/bash

gcc -g main.c binary/structs.h binary/binary.c binary/dealloc.c binary/ls.c binary/remove.c binary/create.c binary/link.c binary/mkdir.c binary/util.c fileio/open.c fileio/readwrite.c fileio/writebinary.c
mv a.out executables/a.out

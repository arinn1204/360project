#!/bin/bash

gcc main.c binary/binary.c binary/dealloc.c binary/ls.c binary/remove.c binary/create.c binary/link.c binary/mkdir.c binary/util.c fileio/open.c fileio/readwrite.c -g

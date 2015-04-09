#ifndef DEALLOC_H
#define DEALLOC_H


#include "binary.h"

//this will allocate a new inode
int ialloc(int dev) {

}

//this will allocate a new block inside a current inode
int balloc(int dev) {

}

//this will deallocate an INODE that is ino
int idealloc(int dev, int ino) {

}

//this will deallocate a BLOCK with block number bno
int iballoc(int dev, int bno) {

}

#endif
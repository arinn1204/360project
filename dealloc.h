#ifndef DEALLOC_H
#define DEALLOC_H


#include "binary.h"


// tests an individual bit to see if it is a one
int test_bit(char *buf, int bit) {
	int i, j;
	i = bit / 8; j = bit % 8;
	if(buf[i] & (1 << j)) {
		return 1;
	}
	return 0;
}

//sets an individual bit to a one
int set_bit(char *buf, int bit) {
	int i=bit/8, j=bit%8;
	buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit) {
	int i=bit/8, j=bit%8;
	buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev) {
	char buf[BLKSIZE];

	getblock(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	putblock(dev, 1, buf);

	getblock(dev, 2, buf);
	gp = (GD *)buf;

	gp->bg_free_inodes_count--;
	putblock(dev, 2, buf);

}

int decFreeBlocks(int dev) {
	char buf[BLKSIZE];

	getblock(dev, 1, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count--;
	putblock(dev, 1, buf);

	getblock(dev, 2, buf);
	gp = (GD *)buf;

	gp->bg_free_blocks_count--;
	putblock(dev, 2, buf);
}

//this will allocate a new inode
int ialloc(int dev) {
	int i;
	char buf[BLKSIZE];

	getblock(dev, imap, buf);

	for (i = 0; i < ninodes) {
		if(test_bit(buf, i) == 0) {
			set_bit(buf, i);
			decFreeInodes(dev);

			putblock(dev, imap, buf);

			return i + 1;
		}
	}

	printf("No more free inodes!\n");
	return 0;

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
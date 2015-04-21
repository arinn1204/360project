#include "structs.h"

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

	getblock(dev, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	sp->s_free_inodes_count--;
	putblock(dev, SUPERBLOCK, buf);

	getblock(dev, GDBLOCK, buf);
	gp = (GD *)buf;

	gp->bg_free_inodes_count--;
	putblock(dev, GDBLOCK, buf);

}

int decFreeBlocks(int dev) {
	char buf[BLKSIZE];

	getblock(dev, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	sp->s_free_blocks_count--;
	putblock(dev, SUPERBLOCK, buf);

	getblock(dev, GDBLOCK, buf);
	gp = (GD *)buf;

	gp->bg_free_blocks_count--;
	putblock(dev, GDBLOCK, buf);
}

//this will allocate a new inode
int ialloc(int dev) {
	int i;
	char buf[BLKSIZE];

	getblock(dev, imap, buf);

	for (i = 0; i < ninodes; i++) {
		if(test_bit(buf, i) == 0) {
			set_bit(buf, i);
			decFreeInodes(dev);

			putblock(dev, imap, buf);
			ninodes--;

			return i + 1;
		}
	}

	printf("No more free inodes!\n");
	return 0;

}

//this will allocate a new block inside a current inode
int balloc(int dev) {
	int i;
	char buf[BLKSIZE];

	getblock(dev, bmap, buf);

	for (i = 0; i < nblocks; i++) {
		if(test_bit(buf, i) == 0) {
			set_bit(buf, i);
			decFreeBlocks(dev);
			nblocks--;

			putblock(dev, bmap, buf);
			return i+1;
		}
	}
	printf("No more free blocks\n");
	return 0;

}

//this will deallocate an INODE, ino
int idealloc(int dev, int ino) {
	char buf[BLKSIZE];

	getblock(dev, imap, buf);

	clr_bit(buf, ino);
	decFreeInodes(dev);

	putblock(dev, imap, buf);
	ninodes++;


}

//this will deallocate a BLOCK, bno
int bdealloc(int dev, int bno) {
	char buf[BLKSIZE];

	getblock(dev, bmap, buf);

	clr_bit(buf, bno);
	decFreeBlocks(dev);

	putblock(dev, bmap, buf);
	nblocks++;

}
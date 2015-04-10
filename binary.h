#ifndef BINARY_H
#define BINARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include "util.h"


//initialize everything

int init(char *name) {
	int i;
	char buf[BLKSIZE];
	for (i = 0; i < NMINODES; i++) {
		minode[i].refcount = 0;
		minode[i].dev = 0;
		minode[i].ino = 0;
		minode[i].dirty = 0;
		minode[i].mounted = 0;
	}
	root = 0;

	for (i = 0; i < NFD; i++) {
		P0->fd[i] = 0;
		P1->fd[i] = 0;
		running->fd[i] = 0;
	}


	P0->next = P1;
	P1->next = P0;
	running->next = P0;

	P0->cwd = 0;
	P1->cwd = 0;
	running->cwd = 0;

}

//initialize ninodes = s_inodes_count
//initialize nblocks = s_blocks_count
//init imap = bg_inode_bitmap
//init bmap = bg_block_bitmap
//init inodeTable = bg_inode_table

int mount_root(char *name) {
	char buf[BLKSIZE];


	fd = open(name, O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Opening device failed\n");
		exit(-1);
	}
	getblock(fd, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	if (sp->s_magic != 0xEF53) {
		fprintf(stderr, "Not EXT2! Exiting now.\n");
		exit(-1);
	}
	printf("Ext2 ... Check\n");
	ninodes = sp->s_inodes_count;
	nblocks = sp->s_blocks_count;

	getblock(fd, GDBLOCK, buf);
	gp = (GD *)buf;
	imap = gp->bg_inode_bitmap;
	bmap = gp->bg_block_bitmap;
	inodeTable = gp->bg_inode_table;

	root = iget(fd, 2);

	P0->cwd = iget(fd, 2);
	P1->cwd = iget(fd, 2);


}

int _ls(char *name) {


}

int _cd(char *name) {

}

int _pwd(char *name) {

}

int _mkdir(char *name) {
}

int _create(char *name) {

}

int _rmdir(char *name) {

}

int _link(char *name) {

}

int _unlink(char *name) {

}

int menu(char *name) {
	printf("[ls][cd][pwd][mkdir][create][rmdir][link][unlink][menu] ");
}

int (*func[10]) (char *name); 


#endif
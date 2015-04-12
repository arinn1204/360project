#ifndef UTIL_H
#define UTIL_H

#include "structs.h"
#include "dealloc.h"

int getblock(int dev, int block, char buf[]) {
	lseek(dev, (long)block*BLKSIZE, 0);
	read(dev, buf, BLKSIZE);
}

int putblock(int dev, int block, char buf[]) {
	lseek(dev, (long)block*BLKSIZE, 0);
	write(dev, buf, BLKSIZE);
}

char *tokenize(char *pathname, char *delim) {
	int len = -1;
	int i, count = 0;
	char *token;

	if(pathname[0] == 0) return;
	len = strlen(pathname);
	char *temp = calloc((len + 1), 1);


	strcpy(temp, pathname);

	for (i=0;i<len;i++) if (temp[i] == *delim) count++;
	nameCount = count + 1;
	if(count == 0) return;
	
	names = calloc(count + 1, 1);
	i=0;
	token = strtok(temp, delim);
	while(token != NULL) {
		names[i] = calloc(strlen(token) + 1, 1);
		strcpy(names[i], token);
		token=strtok(NULL, delim); i++;
	}
	names[i] = 0;
	free(temp);

}

int search(MINODE *mip, char *name, int dev) {
	char c, buf[BLKSIZE], *cp;
	int ret = 0;
	getblock(dev, (mip->inode)->i_block[0], buf);
	dp = (DIR *)buf;
	cp = buf;

	while (cp < buf + BLKSIZE) {
		c = dp->name[dp->name_len];
		dp->name[dp->name_len] = 0;
		if(!strcmp(dp->name, name)) {
			ret = dp->inode;
			dp->name[dp->name_len] = c;
			break;
		}
		dp->name[dp->name_len] = c;
		cp += dp->rec_len;
		dp = (DIR *)cp;
	}

	return ret;


}

int getino(int dev, char *pathname) {
	MINODE *mp = (MINODE *)malloc( sizeof(MINODE) );
	mp->inode = (INODE *)malloc( sizeof(INODE) );
	int i, inumber;
	char buf[BLKSIZE];
	getblock(dev, 2, buf);
	gp = (GD *)buf;
	//inodeTable = gp->bg_inode_table;
	getblock(dev, inodeTable, buf);

	tokenize(pathname, "/");

	mp->inode = (INODE *)buf + 1;
	for (i=0; i < nameCount; i++) {
		inumber = search(mp,*(names+i), dev);
		if(inumber == 0) { printf( "%s was not found!\n", *(names+i) ); return 0; }
		bzero(buf, BLKSIZE);
		getblock(dev, (inumber-1)/8+inodeTable, buf);
		mp->inode = (INODE *)buf + (inumber-1)%8;
	} //end of for loop
	free(mp);
	return inumber;
}

MINODE *iget(int dev, int ino) {
	int i;
	char buf[BLKSIZE];
	for (i = 0; i < NMINODES; i++) {
		if(minode[i].ino == ino) {
			minode[i].refcount++;
			return &minode[i];
		}
	}
	for (i = 0; i < NMINODES; i++) {
		if(minode[i].ino == 0) {
			getblock(dev, (ino-1)/8 + inodeTable, buf);
			minode[i].inode = (INODE *)buf + (ino-1) % 8;
			minode[i].refcount = 1;
			minode[i].dev = dev;
			minode[i].dirty = 1;
			minode[i].ino = ino;
			return &minode[i];
		}
	}
	printf("There is no more space available!\n");
}

int iput(MINODE *mip) {
	int block, inumber;
	char buf[BLKSIZE], *location;
	mip->refcount--;
	if(mip->refcount > 0) return 0;
	if(mip->dirty == 0) return 0;
	else { //the block is dirty (i.e needs to be changed)
			//this is done by reading the block, changing the information
			//then putting the block back to disk
		getblock(mip->dev, (mip->ino - 1)/8 + inodeTable, buf);
		location = buf + ((mip->ino-1) % 8) * 128;
		memcpy(location, (mip->inode), 128);
		putblock(mip->dev, (mip->ino - 1)/8 + inodeTable, buf);
	}

}

int findmyname(MINODE *parent, int myino, char *mynode) {
	char buf[BLKSIZE], c;
	char *cp = buf;
	int i;
	for (i = 0; i < 12; i++) {
		if(parent->inode->i_block[i] != 0) {
			getblock(parent->dev, parent->inode->i_block[i], buf);
			dp = (DIR *)buf;
			while (cp < buf + BLKSIZE) {
				c = dp->name[dp->name_len];
				dp->name[dp->name_len] = 0;
				if(dp->inode == myino) {
					strcmp(mynode, dp->name);
					dp->name[dp->name_len] = c;
					return 0;
				}
				else {
					cp += dp->rec_len;
					dp = (DIR *)cp;
					dp->name[dp->name_len] = c;
				}
			}
		}
	}

	return -1;

}

int findino(MINODE *mip, int *myino, int *parentino) {
	char buf[BLKSIZE];
	getblock(mip->dev, mip->inode->i_block[0], buf);
	myino = (int *)buf;
	parentino = myino + 1;
}

#endif
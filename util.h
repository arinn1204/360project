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
	if(pathname[0] == '/') nameCount--;
	
	if(count == 0) {
		names = calloc(2, 1);
		names[0] = calloc(strlen(pathname) + 1, 1);
		strcpy(*names, pathname);
		return;
	};

	names = calloc(nameCount + 1, 1);
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
	getblock(dev, (mip->inode).i_block[0], buf);
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
	int i, inumber;
	char buf[BLKSIZE];
	getblock(dev, inodeTable, buf);

	tokenize(pathname, "/");
	ip = (INODE *)buf + 1;
	mp->inode = *ip;
	for (i=0; i < nameCount; i++) {
		inumber = search(mp,*(names+i), dev);
		if(inumber == 0) { printf( "%s was not found!\n", *(names+i) ); return 0; }
		bzero(buf, BLKSIZE);
		getblock(dev, INUMBER(inumber-1, inodeTable), buf);
		ip = (INODE *)buf + OFFSET(inumber);
		mp->inode = *ip;
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
		if(minode[i].refcount == 0) {
			getblock(dev, INUMBER(ino, inodeTable), buf);
			ip = (INODE *)buf + OFFSET(ino);
			minode[i].inode = *ip;
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
		getblock(mip->dev, INUMBER(mip->ino, inodeTable), buf);
		location = buf + OFFSET(mip->ino) * 128;
		memcpy(location, &(mip->inode), 128);
		putblock(mip->dev, INUMBER(mip->ino, inodeTable), buf);
	}

}

int findmyname(MINODE *parent, int myino, char **myname) {
	char buf[BLKSIZE], c, temp[256];
	char *cp = buf;
	int i, inumber;

	*ip = parent->inode;
	inumber = INUMBER(parent->ino, inodeTable);
	getblock(parent->dev, inumber, buf);
	ip = (INODE *)buf + OFFSET(parent->ino);


	if(myino == 2) {
		*myname = "/";
	}
	for (i = 0; i < 12; i++) {
		if(ip->i_block[i] != 0) {
			getblock(parent->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			while (cp < buf + BLKSIZE) {
				c = dp->name[dp->name_len];
				dp->name[dp->name_len] = 0;
				if(dp->inode == myino) {
					strcpy(temp, dp->name);
					*myname = temp;
					dp->name[dp->name_len] = c;
					return 1;
				}
				else {
					dp->name[dp->name_len] = c;
					cp += dp->rec_len;
					dp = (DIR *)cp;
				}
			}
		}
	}

	return 0;

}



int findino(MINODE *mip, int *myino, int *parentino) {
	char buf[BLKSIZE], *cp, c;
	int inumber = INUMBER(mip->ino, inodeTable);
	getblock(mip->dev, inumber, buf);
	ip = (INODE *)buf + OFFSET(mip->ino);
	mip->inode = *ip;

	getblock(mip->dev, mip->inode.i_block[0], buf);
	dp = (DIR *)buf;
	cp = buf;

	*myino = dp->inode;
	cp += dp->rec_len;
	dp = (DIR *)cp;
	*parentino = dp->inode;

	
	return 0;
}

int fixPath(char **name) {
	char *temp, temp1[256];
	char *cwd = calloc(20,1);
	MINODE *mp = running->cwd, *fp = running->cwd;
	int child, parent, len = 0, size = 20;
	int i;

	//fp = child
	//mp = parent

	while(1) {
		if (mp != fp) fp = mp;
		findino(fp, &child, &parent);
		mp = iget(fp->dev, parent);
		if(fp->ino == 2) {
			strcpy(temp1, "/");
			strcat(temp1, cwd);
			strcpy(cwd, temp1);
			len++;
			break;
		}
		else {
			findmyname(mp, fp->ino, &temp);
			if(temp == 0) break;
			if(len + 10 >= size) {
				size *= 2;
				cwd = realloc(cwd, size);
			}
			if(cwd[0] == 0) {
				strcpy(cwd, temp);
				strcat(cwd, "/");
				len = strlen(cwd);
			}
			else {
				strcpy(temp1, temp);
				strcat(temp1, "/");
				strcat(temp1, cwd);
				strcpy(cwd, temp1);
				len = strlen(cwd);
			}
		}
		iput(mp);
	}

	if(len + 10 >= size) {
		size *= 2;
		cwd = realloc(cwd, size);
	}

	strcat(cwd, *name);
	strcpy(*name, cwd);
	free(cwd);

	return 1;
}


#endif
#ifndef REMOVE_H
#define REMOVE_H

#include "binary.h"

int _rmdir(char *name) {
	int ino, i;
	MINODE *mip;
	u16 mode;
	char buf[BLKSIZE], *tname, c;


	if(*name == 0) {
		printf("No dir to remove\n");
		return -1;
	}


	tname = (char *)calloc(strlen(name) + 1, 1);
	strcpy(tname, name);
	fixpath(&tname);

	ino = getino(running->cwd->dev, tname);
	if(ino == 0) {
		printf("%s was not found.\n", name);
		return -1;
	}
	mip = iget(running->cwd->dev, ino);

	if(running->uid != 0 && running->uid != mip->inode.i_uid) {
		printf("Permission not allowed.\n");
		return -1;
	}
	mode = mip->inode.i_mode;
	if (!DIR_MODE(mode)) {
		printf("%s is not a directory.\n", name);
		return -1;
	}

	if (mip->inode.i_links_count > 2) {
		printf("%s is not empty.\n", name);
		return -1;
	}
	else {
		ip = &mip->inode;
		for(i = 0; i < 12; i++) {
			if (ip->i_block[i] != 0) {
				getblock(mip->dev, ip->i_block[i], buf);
				dp = (DIR *)buf;
				cp = buf;
				while(cp < buf + BLKSIZE) {
					c = dp->name[dp->name_len];
					dp->name[dp->name_len] = 0;
					if( !strcmp(".", dp->name, strlen(dp->name)) );
					else if ( !strcmp("..", dp->name, strlen(dp->name)) );
					else {
						printf("%s is not empty.\n", name);
						dp->name[dp->name_len] = c;
						return -1;
					}
					dp->name[dp->name_len] = c;
					cp += dp->rec_len;
					dp = (DIR *)cp;

				}
			}
		}
	}

	if (mip->refcount > 1) {
		printf("%s is busy.\n", name);
		return -1;
	}

	




}


int _unlink(char *name) {

}

int _rm(char *fullname) {

}

#endif
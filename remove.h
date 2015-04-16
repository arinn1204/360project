#ifndef REMOVE_H
#define REMOVE_H

#include "dealloc.h"
#include "util.h"


int rm_child(MINODE *pip, char *name) {
	char buf[BLKSIZE], *cp, *np, c;
	int i;
	ip = &pip->inode;

	for(i = 0; i < 12; i++) {
		if(ip->i_block[i] != 0) {
			getblock(pip->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;
			//np is one behind of cp at all times
			while(cp < buf + BLKSIZE) {
				c = dp->name[dp->name_len];
				dp->name[dp->name_len] = 0;

				//found the name to be removed
				if( !strncmp(name, dp->name, dp->name_len) ) {
					dp->name[dp->name_len] = c;

					//the name is the first and only entry
					if (cp == buf + BLKSIZE && np == buf) {
						//dealloc the block that is only this dir
						bdealloc(pip->dev, ip->i_block[i]);
						//move around the blocks so there are no holes
						while (i < 11) {
							if(ip->i_block[i+1] == 0) break;
							ip->i_block[i] = ip->i_block[i + 1];
						}
						ip->i_block[i] = 0;
						return 1;
						//the dir is now removed
					}

					//the name is the very last entry (also pointed to by
					//cp and dp
					else if (cp + dp->rec_len > buf + BLKSIZE) {

					}

					//the name is somewhere in the middle
					else {

					}

					return 1;
				}
				dp->name[dp->name_len] = c;


				np = cp;
				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}
}

int _rmdir(char *name) {
	int ino, pino, i, flag = 0;
	MINODE *mip, *pip;
	u16 mode;
	char buf[BLKSIZE], *tname, *pname, c, *cp;


	if(*name == 0) {
		printf("No dir to remove\n");
		return -1;
	}


	tname = (char *)calloc(strlen(name) + 1, 1);
	strcpy(tname, name);
	fixPath(&tname);

	ino = getino(running->cwd->dev, tname);
	if(ino == 0) {
		printf("%s was not found.\n", name);
		return -1;
	}
	mip = iget(running->cwd->dev, ino);
	mode = mip->inode.i_mode;

	//checking to see if removal is possible
	if(running->uid != 0 && running->uid != mip->inode.i_uid) {
		printf("Permission not allowed.\n");
		flag = 1;
	}
	else if (mip->refcount > 1) {
		printf("%s is busy.\n", name);
		flag = 1;
	}
	else if (!DIR_MODE(mode)) {
		printf("%s is not a directory.\n", name);
		flag = 1;
	}

	else if (mip->inode.i_links_count > 2) {
		printf("%s is not empty.\n", name);
		flag = 1;
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
					if( !strncmp(".", dp->name, dp->name_len) );
					else if ( !strncmp("..", dp->name, dp->name_len) );
					else {
						printf("%s is not empty.\n", name);
						dp->name[dp->name_len] = c;
						flag = 1;
					}
					dp->name[dp->name_len] = c;
					cp += dp->rec_len;
					dp = (DIR *)cp;

				}
			}
		}
	}

	if (flag) {
		iput(mip);
		free(tname);
		return -1;
	}

	//removal is allowed, start removal process

	ip = &mip->inode;

	for(i = 0; i < 12; i++) {
		if(ip->i_block[i] == 0);
		else {
			bdealloc(mip->dev, ip->i_block[i]);
		}
	}
	iput(mip); //puts mip->refcount == 0
	pname = calloc(strlen(tname) + 1, 1);

	strcpy(pname, tname);
	free(tname);

	pname = dirname(pname);

	pino = getino(running->cwd->dev, pname);
	if(pino == 0) {
		printf("%s does not exist\n", pname);
		return -1;
	}

	pip = iget(running->cwd->dev, pino);

	rm_child(pip, name);

	pip->inode.i_links_count--;
	pip->dirty = 1;
	_touch(pname);

	iput(pip);
	return 1;

}

int _rm(char *fullname) {

}


int _unlink(char *name) {

}

#endif
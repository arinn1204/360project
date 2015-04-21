#include "structs.h"


int rm_child(MINODE *pip, char *name) {
	char buf[BLKSIZE], *cp, *np, *pp, c;
	int i, size, j;


	ip = &pip->inode;

	for(i = 0; i < 12; i++) {
		if(ip->i_block[i] != 0) {
			//first loop through np == cp == buf
			//Could not find a way around this
			getblock(pip->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;
			pp = cp;
			np = cp + dp->rec_len;
			//np is one ahead of cp at all times
			while(cp < buf + BLKSIZE) {
				//c = dp->name[dp->name_len];
				//dp->name[dp->name_len] = 0;

				//found the name to be removed
				if( !strncmp(name, dp->name, dp->name_len) ) {
					//dp->name[dp->name_len] = c;

					//the name is the first and only entry
					if ( np == buf + BLKSIZE && cp == buf + 24) {
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

					//cp --> dir to remove
					//np --> dir after it
					else if (cp + dp->rec_len == buf + BLKSIZE) {
						//take recording of the dir to remove's rec length
						size = dp->rec_len;
						dp = (DIR *)pp; //the previous entry
						dp->rec_len += size;

						return 1;
						
					}

					//the name is somewhere in the middle
					else {
						size = dp->rec_len;
						/*****************************
						
						| xxx | yyy | zzz | therestofthestuff
						
							  ^		^
							  cp 	np

						|				BLKSIZE				|
							  |			REMAIN 				|


						remove yyy

						| xxx | zzz | therestofthestuff.reclen + yyy.rec_len

							  ^
							  cp 			np is now in the abyss
	
						******************************/
						//memcpy(dest, source, numberofbytes);
						memcpy(cp, np, &buf[BLKSIZE-1]-np);
						dp = (DIR *)cp;							
						while (cp + dp->rec_len + size < buf + BLKSIZE) {
							cp += dp->rec_len;
							dp = (DIR *)cp;
						}

						dp->rec_len += size;

					}

					return 1;
				}


				pp = cp;
				cp += dp->rec_len;


				dp = (DIR *)cp;
				np = cp + dp->rec_len;
			}
		}

		return 0;
	}
}

int _rmdir(char *name) {
	int ino, pino, cino, i, flag = 0;
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
	cino = mip->ino;
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
	name = basename(name);

	rm_child(pip, name);

	idealloc(pip->dev, cino);
	pip->inode.i_links_count--;
	pip->dirty = 1;
	_touch(pname);

	iput(pip);
	return 1;

}

int _unlink(char *name) {
	int mino, pino, temp;
	MINODE *mip, *pip;
	char *childName, *parentName;

	if (*name == 0) {
		printf("Enter a name to unlink");
		return -1;
	}


	fixPath(&name);
	mino = getino(running->cwd->dev, name);

	if (mino == 0) {
		printf("%s does not exist\n", name);
		return -1;
	}

	mip = iget(running->cwd->dev, mino);
	ip = &mip->inode;

	ip->i_links_count--;
	if (ip->i_links_count == 0) {
		//need to remove all datablocks, function found in util.h
		truncateI(ip, mip->dev);
		idealloc(mip->dev, mino);
	}
	childName = basename(name);
	parentName = dirname(name);

	pino = getino(mip->dev, parentName);

	if(pino == 0) {
		printf("%s does not exist\n", parentName);
		return -1;
	}
	

	pip = iget(mip->dev, pino);

	return rm_child(pip, childName);


}
int _rm(char *fullname) {
	_unlink(fullname);
}

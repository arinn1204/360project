#ifndef CREATE_H
#define CREATE_H

#include "mkdir.h"

int mycreate(MINODE *pip, char *name) {
	int ino, bno, i;
	char buf[BLKSIZE], *cp = buf;
	dp = (DIR *)buf;
	MINODE *mip;

	ino = ialloc(pip->dev);
	//bno = balloc(pip->dev);

	if (ino == 0) {
		return -1;
	}

	mip = iget(pip->dev, ino);

	ip = &mip->inode;
	ip->i_mode = FILE_PERMISSION;
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = 0;
	ip->i_links_count = 1;
	ip->i_atime = time(0L);
	ip->i_mtime = time(0L);
	ip->i_ctime = time(0L);
	ip->i_blocks = 0;

	for (i = 0; i < 12; i++) {
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;
	iput(mip);

	entername(pip, ino, name);
}

int _creat(char *name) {
	int child, parent, cino, pino;
	MINODE *cip, *pip;
	char *pname;
	u16 mode;

	if (*name == 0) {
		printf("Create: No file name\n");
		return -1;
	}

	if (name[0] != '/') fixPath(&name);

	pname = (char *)calloc(strlen(name) + 1, 1);
	strcpy(pname, name);
	pname = dirname(pname);

	pino = getino(&running->cwd->dev, pname);
	if (pino == 0) {
		printf("%s was not found.\n", pname);
		return 0;
	}
	pip = iget(running->cwd->dev, pino);
	mode = pip->inode.i_mode;
	if( ! DIR_MODE(mode) ) {
		printf("%s is not a directory\n", pname);
		iput(pip);
		return -1;
	}

	if ( getino(&pip->dev, name) ) {
		printf("Cannot create file. File already exists\n");
		iput(pip);
		return -1;
	}

	name = basename(name);
	if (mycreate(pip, name) == -1) {
		iput(pip);
		free(pname);
		return 0;
	}


	ip = &pip->inode;

	ip->i_atime = time(0L);
	ip->i_mtime = time(0L);
	

	pip->dirty = 1;
	iput(pip);
	free(pname);

	return 1;
}


int _touch(char *name) {
	MINODE *pip;
	char *pname;
	int pino;
	u16 mode;


	if (*name == 0) {
		printf("Create: No file name\n");
		return -1;
	}

	if (name[0] != '/') fixPath(&name);

	pname = (char *)calloc(strlen(name) + 1, 1);
	strcpy(pname, name);
	pname = dirname(pname);

	pino = getino(&running->cwd->dev, pname);
	if (pino == 0) {
		printf("%s was not found.\n", pname);
		return -1;
	}
	pip = iget(running->cwd->dev, pino);
	mode = pip->inode.i_mode;

	if( ! DIR_MODE(mode) ) {
		printf("%s not a directory\n", pname);
		return -1;
	}

	if ( getino(&pip->dev, name) == 0 ) {
		_creat(name);
		iput(pip);
	}
	else {
		ip = &pip->inode;
		ip->i_atime = time(0L);
		ip->i_mtime = time(0L);
		pip->dirty = 1;
		iput(pip);
	}



	free(pname);

	return 1;

}

#endif
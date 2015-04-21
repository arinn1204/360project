#include "structs.h"


int entername(MINODE *pip, int myino, char *name) {
	int i, blk, remain, ideal;
	char buf[BLKSIZE], *cp;

	for (i = 0; i < 12; i++) {
		if (pip->inode.i_block[i] == 0)
			break;
		getblock(pip->dev, pip->inode.i_block[i], buf);

		dp = (DIR *)buf;
		cp = buf;
		blk = pip->inode.i_block[i];

		while(cp + dp->rec_len < buf + BLKSIZE) {
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
		// dp is now at the last available block
	}

	ideal = 4 * ( (8 + dp->name_len + 3) / 4);
	remain = dp->rec_len - ideal;

	if (remain >= ideal) {
		dp->rec_len = ideal;
		cp += dp->rec_len;
		dp = (DIR *)cp;
		dp->inode = myino;
		dp->rec_len = remain;
		dp->name_len = strlen(name);

		memcpy((char *)dp->name, name, strlen(name));
		putblock(pip->dev, blk, buf);
				
	}
	else {
		blk = balloc(pip->dev);
		pip->inode.i_size += 1024;
		pip->inode.i_block[i] = blk;
		dp = (DIR *)buf;
		dp->inode = myino;
		dp->rec_len = BLKSIZE;
		dp->name_len = strlen(name);
		memcpy((char *)dp->name, name, strlen(name));

		putblock(pip->dev, blk, buf);
	}
}

int mymkdir(MINODE *parent, char *dirName) {
	//allocating new block and inode for the directory
	int ino, bno, i;
	char buf[BLKSIZE], *cp = buf;
	//char *dot = ".", *dotdot = "..";
	dp = (DIR*)buf;

	MINODE *mip; //inode that we are loading information into

	ino = ialloc(parent->dev);
	bno = balloc(parent->dev);

	if(ino == 0 || bno == 0) {
		return -1;
	}

	mip = iget(parent->dev, ino);

	ip = &mip->inode;
	ip->i_mode = 0x41ED;
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = BLKSIZE;
	ip->i_links_count = 2;
	ip->i_atime = time(0L);
	ip->i_mtime = time(0L);
	ip->i_ctime = time(0L);
	ip->i_blocks = 2;
	ip->i_block[0] = bno;

	for(i = 1; i < 12; i++) {
		ip->i_block[i] = 0;
	}

	mip->dirty = 1;
	iput(mip);

	dp->inode = ino;
	dp->rec_len = 12;
	dp->name_len = 1;
	//strcpy(dp->name, ".");
	memcpy(dp->name, ".", 1);

	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode = parent->ino;
	dp->rec_len = 1012;
	dp->name_len = 2;
	//strcpy(dp->name, "..");
	memcpy(dp->name, "..", 2);


	putblock(running->cwd->dev, bno, buf);

	entername(parent, ino, dirName);


}

int _mkdir(char *name) {
	int child, parent, cino, pino;
	MINODE *cip, *pip;  //child inode ptr and parent inode ptr
	char *pname; //parents name
	u16 mode;

	if (*name == 0) {
		printf("mkdir: Missing file\n");
		return -1;
	}

	//changing the path name to absolute (if it is not already absolute)
	if (name[0] != '/') fixPath(&name);

	pname = (char *)calloc(strlen(name) + 1, 1);
	strcpy(pname, name);
	dirname(pname);
	name = basename(name);

	pino = getino(running->cwd->dev, pname);
	if(pino == 0) {
		printf("%s was not found.\n", pname);
		return 0;
	}
	pip = iget(running->cwd->dev, pino);
	mode = pip->inode.i_mode;
	if (! DIR_MODE(mode)) {
		printf("%s is not a directory\n", pname);
		return -1;
	}

	if ( getino(pip->dev, name) ) {
		printf("Cannot create dir, already exists.\n");
		return -1;
	}



	if(mymkdir(pip, name) == -1) {
		iput(pip);
		free(pname);
		return 0;
	}

	pip->inode.i_links_count++;
	pip->inode.i_atime = time(0L);
	pip->inode.i_mtime = time(0L);
	pip->dirty = 1;
	iput(pip);
	free(pname);

	return 1;

}
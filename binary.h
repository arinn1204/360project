#ifndef BINARY_H
#define BINARY_H

#include "util.h"


int destruct() {
	free(P0->next);
	free(P1->next);
	free(running->next);

	free(P0->cwd);
	free(P1->cwd);
	free(running->cwd);

	free(P0);
	free(P1);
	free(running);
}


//initialize everything

int init(char *name) {
	int i;
	char buf[BLKSIZE];

	P0 = (PROC *)malloc( sizeof(PROC) );
	P1 = (PROC *)malloc( sizeof(PROC) );
	running = (PROC *)malloc( sizeof(PROC) );


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

	P0->next = (PROC *)malloc( sizeof(PROC *) );
	P1->next = (PROC *)malloc( sizeof(PROC *) );
	running->next = (PROC *)malloc( sizeof(PROC *) );

	P0->next = P1;
	P1->next = P0;
	running->next = P0;


	P0->cwd = (MINODE *)malloc( sizeof(MINODE) );
	P1->cwd = (MINODE *)malloc( sizeof(MINODE) );
	running->cwd = (MINODE *)malloc( sizeof(MINODE) );


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
	if (sp->s_magic != SUPER_MAGIC) {
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

	//this sets the running process (PROC *) to P0
	running = P0;


}

int list_file(MINODE *mip, char *name) {
	char *t1 = "xwrxwrxwr";
	char ftime[64];

	int i, foo;

	*ip = mip->inode;

	u16 mode = ip->i_mode;
	u16 link = ip->i_links_count;
	u16 uid = ip->i_uid;
	u16 gid = ip->i_gid;
	u16 size = ip->i_size;

	if(DIR_MODE(mode))  printf("d");
	else if(FILE_MODE(mode)) printf("-");
	else if(LINK(mode)) printf("l");

	for (i = 8; i >= 0; i--) {
		if(mode & (1 << i))
			putchar( *(t1 + i) );
		else
			putchar('-');
	}

	printf(" %d", link);
	printf(" %d", uid);
	printf(" %d", gid);
	printf(" %d", size);
	//strcpy(ftime, (char)ctime( &mip->inode.i_ctime ) );
	//printf(" %s", ftime);
	printf(" %s", name);
	if (LINK(mode)) {
		printf(" -> %s", (char *)ip->i_block);
	}
	printf("\n");

}

int list_dir(MINODE *mip) {
	struct dirent *foo;
	char buf[BLKSIZE], *cp, c;
	MINODE *cip;
	int i;

	*ip = mip->inode;
	getblock(mip->dev, INUMBER(mip->ino, inodeTable), buf);
	ip = (INODE *)buf + OFFSET(mip->ino);
	for (i = 0; i < 12; i++) {
		if (ip->i_block[i] != 0) {
			getblock(mip->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;
			while(cp < buf + BLKSIZE) {	
				if(dp->name[0] == '.');
				else {
					c = dp->name[dp->name_len];
					dp->name[dp->name_len] = 0;

					printf("I_number\tRec_len\t\tName_len\tName\n");
					printf("%d\t\t%d\t\t%d\t\t%s\n",
						dp->inode, dp->rec_len, dp->name_len, dp->name);


					cip = iget(mip->dev, dp->inode);
					list_file(cip, dp->name);

					iput(cip);
					dp->name[dp->name_len] = c;
				}
				cp += dp->rec_len;
				dp = (DIR *)cp;
				printf("\n");
			}
		}
	}


}

int _ls(char *name) {
   int ino, child, parent;
   MINODE *mip; 

   //finding the name of the current dir (as to give a full path for LS to function correctly)
   if (*name == 0) {
   		findino(running->cwd, &child, &parent);
   		mip = iget(running->cwd->dev, parent);
   		findmyname(mip, running->cwd->ino, &name);

   		fixPath(&name);
   		//at this point name should be the entire path

   		//no more need for the mip MINODE, so return it.
   		iput(mip);
   }

   else if (name[0] != '/') {
   	//this case is that the name is a relative path, so we will turn this into a absolute
   	fixPath(&name);
   }

   //no case needed if absolute path is already given.


   ino = getino(fd, name);
   if (ino == 0) {
   	printf("%s was not found.\n");
   	return 0;
   }
   mip = iget(fd,ino);
   *ip = mip->inode;

   if (FILE_MODE(ip->i_mode))
      list_file(mip, basename(name));
   else
      list_dir(mip);

   iput(mip);

}
int _cd(char *name) {
	int ino, child, parent;
	int flag = 0;
	MINODE *mip;
	char *path, *myname;
	if(name[0] == 0) {
		path = calloc(2,1);
		strcpy(path, "/");
		ino = 2;
	}
	else if (name[0] != '/') { 
		//relative path
		strcpy(path, name);
		fixPath(&path);
		ino = getino(fd, path);
		if (ino == 0) {
			printf("%s was not found.\n", path);
			return -1;
		}

	}
	else {
		path = calloc(strlen(name) + 1, 1);
		flag = 1;
		strcpy(path, name);
		ino = getino(fd, path);
		if (ino == 0) {
			printf("%s was not found.\n");
			return -1;
		}
	}

	mip = iget(fd, ino);

	if (FILE_MODE(mip->inode.i_mode)) {
		printf("%s is not a directory\n", path);
		if (flag)	free(path);
	}
	else {
		iput(running->cwd);
		running->cwd = mip;
		if (flag) free(path);
	}	
	return 0;


}

int _pwd(char *name) {
	char *temp, temp1[256];
	char *cwd = calloc(20,1);
	int child, parent, len = 0, size = 20;
	int i;

	MINODE *mp = iget(running->cwd->dev, running->cwd->ino);
	MINODE *fp = mp;

	//fp = child
	//mp = parent

	while(1) {
		if (mp != fp) fp = mp;
		findino(fp, &child, &parent);
		mp = iget(fp->dev, parent);
		if(fp->ino == 2) {
			printf("/");
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
	cwd[len-1] = 0;

	printf("%s\n", cwd);


	return 0;
}

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
	strcpy(dp->name, ".");

	cp += dp->rec_len;
	dp = (DIR *)cp;

	dp->inode = parent->ino;
	dp->rec_len = 1012;
	dp->name_len = 2;
	strcpy(dp->name, "..");

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



	if(mymkdir(pip, name) == -1) return 0;

	pip->inode.i_links_count++;
	pip->dirty = 1;
	iput(pip);
	free(pname);

	return 1;

}

int _creat(char *name) {

}

int _rmdir(char *name) {

}

int _link(char *name) {

}

int _unlink(char *name) {

}

int _rm(char *fullname) {

}
int _symlink(char *source) {

}

int _chmod(char *name) {

}

int _chown(char *name) {

}

int _stat(char *file) {

}

int _touch(char *filename) {

}

int menu(char *name) {
	printf("[ls][cd][pwd][mkdir][create] ");
}

int quit(char *name) {
	exit(-1);
}

int (*func[32]) (char *name) = {init, mount_root, _ls, _cd, _pwd,
								_mkdir, _creat, _rmdir, _link, _unlink,
								_rm, _symlink,_chmod, _chown, _stat,
								_touch,0,0,0,0,
								0,0,0,0,0,
								0,0,0,0,0,
								quit, menu};

#endif
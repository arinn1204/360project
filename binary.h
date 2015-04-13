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

	int i;

	*ip = mip->inode;



	if(DIR_MODE(ip->i_mode))  printf("d");
	if(FILE_MODE(ip->i_mode)) printf("-");
	if(LINK(ip->i_mode)) printf("l");

	for (i = 8; i >= 0; i--) {
		if(ip->i_mode & (1 << i))
			putchar( *(t1 + i) );
		else
			putchar('-');
	}

	printf(" %d", ip->i_links_count);
	printf(" %d", ip->i_uid);
	printf(" %d", ip->i_gid);
	printf(" %d", ip->i_size);
	//strcpy(ftime, (char*)ctime( ip->i_ctime ) );
	//printf(" %s", ip->i_ctime);
	printf(" %s", name);
	if (LINK(ip->i_mode)) {
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
	printf("\tI_number\tRec_len\t\tName_len\tName\n");
	for (i = 0; i < 12; i++) {
		if (ip->i_block[i] != 0) {
			getblock(mip->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;
			while(cp < buf + BLKSIZE) {
				printf("\t%d\t\t%d\t\t%d\t\t%s\n",
				dp->inode, dp->rec_len, dp->name_len, dp->name);
				c = dp->name[dp->name_len];
				dp->name[dp->name_len] = 0;
				cip = iget(mip->dev, dp->inode);
				list_file(cip, dp->name);

				iput(cip);
				dp->name[dp->name_len] = 0;


				cp += dp->rec_len;
				dp = (DIR *)cp;
			}
		}
	}


}

int _ls(char *name) {
   int ino, child, parent;
   MINODE *mip 

   //finding the name of the current dir (as to give a full path for LS to function correctly)
   if (name[0] == 0) {
   		findino(running->cwd, &child, &parent);
   		mip = iget(running->cwd->dev, parent);
   		findmyname(mip, running->cwd->ino, &name);

   		fixPath(&name);
   		//at this point name should be the entire path

   		//no more need for the mip MINODE, so return it.
   		iput(mip);
   }


   ino = getino(fd, name);
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
		if (ino == 0) return -1;

	}
	else {
		path = calloc(strlen(name) + 1, 1);
		flag = 1;
		strcpy(path, name);
		ino = getino(fd, path);
		if (ino == 0) return -1;
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

int _mkdir(char *name) {
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
	printf("[ls][cd][pwd][mkdir][create][rmdir][link][unlink] ");
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
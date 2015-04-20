#ifndef BINARY_H
#define BINARY_H

#include "util.h"
#include "create.h"
#include "ls.h"
#include "remove.h"
#include "link.h"
#include "../fileio/open.h"


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
	running->uid = getuid();
	running->pid = getpid();
	running->gid = getgid();

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
	else if ( !strncmp(".", name, strlen(name)) ) {
		findino(running->cwd, &child, &parent);
		ino = child;

	}
	else if ( !strncmp("..", name, strlen(name)) ) {
		findino(running->cwd, &child, &parent);
		ino = parent;
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
		if (flag) free(path);
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

int _chmod(char *name) {

}

int _chown(char *name) {

}

int _stat(char *file) {

}

int menu(char *name) {
	printf("[ls][cd][pwd]\n"
		"[mkdir][create][touch][link][symlink]\n"
		"[rmdir][rm][unlink]\n");
}

int quit(char *name) {
	//destruct();
	exit(1);
}

int (*func[32]) (char *name) = {init, mount_root, _ls, _cd, _pwd,
								_mkdir, _creat, _rmdir, _link, _unlink,
								_rm, _symlink,_chmod, _chown, _stat,
								_touch,0,0,0,0,
								0,0,0,0,0,
								0,0,0,0,0,
								quit, menu};

#endif
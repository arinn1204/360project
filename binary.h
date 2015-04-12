#ifndef BINARY_H
#define BINARY_H

#include "util.h"


int destruct() {
	free(P0->next);
	free(P1->next);
	free(running->next);

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
	if (sp->s_magic != 0xEF53) {
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


}

int list_file(MINODE *mip, char *name) {
	char *t1 = "xwrxwrxwr";
	char *t2 = "---------";
	char ftime[64];

	int i;

	ip = mip->inode;

	if((ip->i_mode & 0xF000) == 0x4000) printf("d");
	if((ip->i_mode & 0xF000) == 0x8000)	printf("-");
	if((ip->i_mode & 0xF000) == 0xA000) printf("l");

	for (i = 8; i >= 0; i--) {
		if(ip->i_mode & (1 << i))
			printf("%c", t1[i]);
		else
			printf("%c", t2[i]);
	}

	printf(" %d", ip->i_links_count);
	printf(" %d", ip->i_uid);
	printf(" %d", ip->i_gid);
	printf(" %d", ip->i_size);
	//strcpy(ftime, (char*)ctime( ip->i_ctime ) );
	printf(" %s", ip->i_ctime);
	printf(" %s", name);
	if ((ip->i_mode & 0xF000) == 0xA000) {
		printf(" -> %s", (char *)ip->i_block);
	}
	printf("\n");

}

int list_dir(MINODE *mip) {
	struct dirent *foo;
	MINODE *cip;
	ip = mip->inode;


}

int _ls(char *name) {
   int ino = getino(fd, name);
   MINODE *mip = iget(fd,ino);

   if ((mip->inode->i_mode & 0xF000) == 0x8000)
      list_file(mip, basename(name));
   else
      list_dir(mip);

   iput(mip);

}

int _cd(char *name) {

}

int _pwd(char *name) {


}

int _mkdir(char *name) {
}

int _create(char *name) {

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

int (*func[32]) (char *name) = {init, mount_root, _ls, _cd, _pwd, _mkdir, _create, _rmdir, _link, _unlink, _rm, _symlink,
									_chmod, _chown, _stat, _touch, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,quit, menu};

#endif
#ifndef STRUCTS_H
#define STRUCTS_H

#define BLKSIZE 1024

//Block Numbers for EXT2
#define SUPERBLOCK 	1
#define GDBLOCK 	2
#define ROOT_INODE 	2

//Dir and Reg file modes
#define SUPER_MAGIC 0xEF53
#define SUPER_USER  0


//Macros to determine: Directory, File, or symlink
#define DIR_MODE(z) (z & 0xF000) == 0x4000
#define FILE_MODE(z) (z & 0xF000) == 0x8000
#define LINK(z) (z & 0xF000) == 0xA000

//Proc Status
#define FREE 	0
#define READY 	1
#define RUNNING 2

//Table sizes
#define	NMINODES	100
#define NMOUNT		 10
#define NPROC		 10
#define NFD			 10
#define NOFT		100


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>



typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR;

GD *gp;
SUPER *sp;
INODE *ip;
DIR *dp;


//In-memory Inode Structure
struct minode {
	INODE *inode;
	int refcount;
	int dev;
	int ino;
	int dirty;
	int mounted;
	struct mount *mountptr;
};

//Open File Table
struct oft {
	int mode;
	int refcount;
	struct minode *inodeptr;
	int offset;
};

struct mount {
	int dev;
	int nblock, ninode;
	int bmap, imap, iblk;
	struct minode *mounted_inode;
	char name[64];
	char mount_name[64];
};

struct proc {
	struct proc *next;
	int uid;
	int pid, gid;
	int status;
	struct minode *cwd;
	struct oft *fd[NFD];	
};

typedef struct proc PROC;
typedef struct minode MINODE;
typedef struct oft OFT;
typedef struct mount MOUNT;


//The MINODE structures
MINODE minode[NMINODES], *root;


char **names; //array of strings for the whole path
//int name[64]; 		//array of pointers to the address of the strings
int nameCount = -1;		//number of items in the arrays
/*
	inodeTable  = The inode start block
	imap 	    = shows all of the inodes in the FS
	bmap		= shows all of the blocks in the FS
	ninodes     = Number of inodes available
	nblocks		= Number of blocks available

*/
int inodeTable = -1, imap = 0, bmap = 0, ninodes = 0, nblocks = 0;

//This is for the descriptor
int fd;


//The three processes that will be running
PROC *P0, *P1, *running;


char pathname[64], parameter[64], cmd[32];

#endif
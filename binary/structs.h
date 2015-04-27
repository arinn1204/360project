#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>



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

//Default link permissions (1010000111111111)
#define LINK_MODE 0xA1FF

//Default File permissions (1000000110100100)
#define FILE_PERMISSION 0x81A4


//Mailmans Algorithm
#define INUMBER(y,z) (y - 1) / 8 + z
#define OFFSET(y) (y - 1) % 8


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
	INODE inode;
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
typedef short unsigned int u16;
typedef unsigned int u32;
typedef unsigned long int u64;

//The user defined structs/////////////////////////////

//Minode structs
MINODE minode[NMINODES], *root;

//The three processes that will be running
PROC *P0, *P1, *running;

//The mount array
MOUNT MT[NMOUNT];

char **names; //array of strings for the whole path
//int name[64]; 		//array of pointers to the address of the strings
int nameCount;		//number of items in the arrays
/*
	inodeTable  = The inode start block
	imap 	    = shows all of the inodes in the FS
	bmap		= shows all of the blocks in the FS
	ninodes     = Number of inodes available
	nblocks		= Number of blocks available

*/
int inodeTable, imap, bmap, ninodes, nblocks ;

//This is for the root device number (should be 3)
int fd;

char pathname[64], parameter[64], cmd[32];

#endif
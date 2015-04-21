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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>



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

//The MINODE structures
MINODE minode[NMINODES], *root;


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

//This is for the descriptor
int fd;


//The three processes that will be running
PROC *P0, *P1, *running;


char pathname[64], parameter[64], cmd[32];

/***********************************************************************

				FUNCTION PROTOTYPES

************************************************************************/




/********************************************************

				UTILITY FUNCTIONS

*********************************************************/

int getblock(int dev, int block, char buf[]);
int putblock(int dev, int block, char buf[]);
char *tokenize(char *pathname, char *delim);
int search(MINODE *mip, char *name, int dev);
int getino(int dev, char *pathname);
MINODE *iget(int dev, int ino);
int iput(MINODE *mip);
int findmyname(MINODE *parent, int myino, char **myname);
int findino(MINODE *mip, int *myino, int *parentino);
int fixPath(char **name);
int truncateI(INODE *inode, int dev);
int test_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);
int decFreeInodes(int dev);
int decFreeBlocks(int dev);
int ialloc(int dev);
int balloc(int dev);
int idealloc(int dev, int ino);
int bdealloc(int dev, int bno);


/********************************************************

				INIT FUNCTIONS

*********************************************************/

//int destruct();
int init(char *name);
int mount_root(char *name);


/********************************************************

				FILE/DIR REMOVAL

*********************************************************/
int rm_child(MINODE *pip, char *name);
int _rmdir(char *name);
int _unlink(char *name);
int _rm(char *fullname);

/********************************************************

				FILE/DIR CREATION

*********************************************************/
int _link(char *source);
int _symlink(char *source);
int mycreate(MINODE *pip, char *name);
int _creat(char *name);
int _touch(char *name);
int entername(MINODE *pip, int myino, char *name);
int mymkdir(MINODE *parent, char *dirName);
int _mkdir(char *name);

/********************************************************

				LIST FILE/DIR AND ETC FUNCTIONS

*********************************************************/
int list_file(MINODE *mip, char *name);
int list_dir(MINODE *mip);
int _ls(char *name);
int _cd(char *name);
int _pwd(char *name);
int _chmod(char *name);
int _chown(char *name);
int _stat(char *name);
int menu(char *name);
int quit(char *name);


/********************************************************

					FILE I/O AND FILE FUNCTIONS

*********************************************************/
int _read(int dev, char *obuf, int bytes);
int _write(int dev, char *ibuf, int bytes);
OFT *getOFT(int mode, MINODE *mip, int offset, int *des);
int openValue(int ino);
int getMode(char *mode);
int _open(char *name, char *mode);
int _close(int dev);

/********************************************************

					FUNCTION TABLE INIT

*********************************************************/

int (*func[32]) (char *name) = {init, mount_root, _ls, _cd, _pwd,
								_mkdir, _creat, _rmdir, _link, _unlink,
								_rm, _symlink,_chmod, _chown, _stat,
								_touch,0,0,0,0,
								0,0,0,0,0,
								0,0,0,0,0,
								quit, menu};

#endif
#ifndef MOUNT_H
#define MOUNT_H

#include "../binary/structs.h"
#include "../binary/util.h"
#include "../binary/binary.h"

int isMounted(char *name, int *i) {

	for (*i = 0; *i < NMOUNT; *i++) {
		if (MT[*i].dev == 0) break;
		else if ( !strncmp(MT[*i].name, name, strlen(name) ))
			return -1;
	}

	return 0;
}

void fillFields(MOUNT *mp) {
	char buf[BLKSIZE];
	getblock(mp->dev, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	mp->ninode = sp->s_inodes_count;
	mp->nblock = sp->s_blocks_count;

	getblock(mp->dev, GDBLOCK, buf);
	gp = (GD *)buf;
	mp->imap = gp->bg_inode_bitmap;
	mp->bmap = gp->bg_block_bitmap;
	mp->iblk = gp->bg_inode_table;

}

MOUNT *getMount(int dev, char *filesys, char *name, MINODE *mip) {
	int i;

	for (i = 0; i < NMOUNT; i++) {
		if (MT[i].dev == 0) {
			MT[i].dev = dev;
			//MT[i].name = (char *)calloc(strlen(filesys) + 1, 1);
			//MT[i].mount_name = (char *)calloc(strlen(name) + 1, 1);
			memcpy(MT[i].name, filesys, strlen(filesys) );
			memcpy(MT[i].mount_name, name, strlen(name) );

			MT[i].mounted_inode = (MINODE *)malloc( sizeof(mip) );
			memcpy(MT[i].mounted_inode, mip, sizeof(mip) );
			fillFields(&MT[i]);
			return &MT[i];
		}
	}
}

int isEXT2(int dev) {
	char buf[BLKSIZE];

	getblock(dev, SUPERBLOCK, buf);
	sp = (SUPER *)buf;
	if (sp->s_magic != SUPER_MAGIC) {
		printf("Not EXT2!\n");
		return -1;
	}

	return 0;

}


int mount(char *name) {
	char *temp;
	int dev, ino, i;
	MINODE *mount_point;
	u16 mode;
	MOUNT *newMount;

	if ( !*name && !*pathname) {
		//no parameters, print mount table
		for (i = 0; i < NMOUNT; i++) {
			if (MT[i].dev == 0) break;
			else {
				printf( "%d: Name = %s MountPoint = %s\n"
						"Dev = %d, bmap = %d, imap = %d\n"
						"nblock = %d, ninode = %d, iblk = %d\n"
						"Mounted Inode = 0x%8x\n",
						i, MT[i].name, MT[i].mount_name, MT[i].dev,
						MT[i].bmap, MT[i].imap, MT[i].nblock, MT[i].ninode,
						MT[i].iblk, MT[i].mounted_inode
					);
			}
		}

		return 0;
	}
	else if (*name == 0 && *pathname != 0) {
		printf("No directory to mount on.\n");
		return -1;
	}


	if (*name == '/') fixPath(&name);
	temp = (char *)calloc(strlen(parameter) + 1, 1);
	strcpy(temp, parameter);
	if (*temp == '/') fixPath(&temp);
	strcpy(parameter, temp);

	if (isMounted(name, &i) == -1) {
		printf("%s is already mounted.\n", name);
		return -1;
	}

	dev = open(name, O_RDWR);
	if( isEXT2(dev) == -1 ) {
		return -1;
	}

	ino = getino(&running->cwd->dev, temp);
	if (ino == 0) {
		printf("%s does not exist.\n", temp);
		return -1;
	}

	mount_point = iget(running->cwd->dev, ino);
	mode = mount_point->inode.i_mode;

	if ( !DIR_MODE(mode) ) {
		printf("Must mount to a dir.\n");
		return -1;
	}
	if (mount_point->refcount > 1) {
		printf("Dir is busy\n");
		return -1;
	}

	newMount = getMount(dev, name, temp, mount_point);


	mount_point->mountptr = (MOUNT *)malloc( sizeof(newMount) );
	memcpy(mount_point->mountptr, newMount, sizeof(newMount) );
	mount_point->mounted = 1;
	mount_point->dirty = 1;



	free(temp);
	return 0;
}



#endif

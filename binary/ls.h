#ifndef LS_H
#define LS_H

#include "structs.h"
#include <grp.h>
#include <pwd.h>

int list_file(MINODE *mip, char *name) {
	char *t1 = "xwrxwrxwr";
	char ftime[64];
	char tname[64];

	struct passwd *pw;
	struct group *gr;

	strcpy(tname, name);

	int i, foo;

	*ip = mip->inode;

	u16 mode = ip->i_mode;
	u16 link = ip->i_links_count;
	u16 uid = ip->i_uid;
	u16 gid = ip->i_gid;
	u32 size = ip->i_size;
	const time_t *ct = &ip->i_ctime;

	pw = getpwuid(uid);
	gr = getgrgid(gid);

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
	printf(" %s", pw->pw_name);
	printf(" %s", gr->gr_name);
	printf(" %d", size);
	//strcpy(ftime, (char *)ctime( ct ) );
	//ftime[strlen(ftime) - 1] = 0;
	//printf(" %s", ftime);
	printf(" %s", tname);
	if (LINK(mode)) {
		printf(" -> %s", (char *)ip->i_block);
	}
	printf("\n");

}

int list_dir(MINODE *mip) {
	char buf[BLKSIZE], *cp, c;
	MINODE *cip;
	int i;

	*ip = mip->inode;
	getblock(mip->dev, INUMBER(mip->ino, inodeTable), buf);
	ip = (INODE *)buf + OFFSET(mip->ino);
	for (i = 0; i < 12; i++) {
		if (mip->inode.i_block[i] != 0) {
			getblock(mip->dev, ip->i_block[i], buf);
			dp = (DIR *)buf;
			cp = buf;
			while(cp < buf + BLKSIZE) {	
				c = dp->name[dp->name_len];
				dp->name[dp->name_len] = 0;

				printf("I_number\tRec_len\t\tName_len\tName\n");
				printf("%d\t\t%d\t\t%d\t\t%s\n",
					dp->inode, dp->rec_len, dp->name_len, dp->name);


				cip = iget(mip->dev, dp->inode);
				list_file(cip, dp->name);

				dp->name[dp->name_len] = c;
				iput(cip);
				
				cp += dp->rec_len;
				dp = (DIR *)cp;
				printf("\n");
			}
		}
	}


}

int _ls(char *name) {
   int ino, child, parent, dev, i, newdev = running->cwd->dev;
   MINODE *mip, *pip; 

   //finding the name of the current dir (as to give a full path for LS to function correctly)
    if (*name == 0) {
   		findino(running->cwd, &child, &parent);
   		mip = iget(newdev, parent);
   		findmyname(mip, running->cwd->ino, &name);

   		if (*name != '/') fixPath(&name);
   		//at this point name should be the entire path

   		//no more need for the mip MINODE, so return it.
   		iput(mip);
    }

    else if (name[0] != '/') {
   	//this case is that the name is a relative path, so we will turn this into a absolute
   		fixPath(&name);
    }

   //no case needed if absolute path is already given.

    ino = getino(&newdev, name);
    if (ino == 0) {
	   	printf("%s was not found.\n");
	   	return 0;
    }

	mip = iget(newdev,ino);

    if (FILE_MODE(mip->inode.i_mode))
       list_file(mip, basename(name));
    else
       list_dir(mip);
 
    iput(mip);

}

#endif
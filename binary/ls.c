#ifndef LS_H
#define LS_H


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
		if (mip->inode.i_block[i] != 0) {
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

					dp->name[dp->name_len] = c;
					iput(cip);
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

#endif
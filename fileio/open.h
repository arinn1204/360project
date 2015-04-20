#ifndef OPEN_H
#define OPEN_H

OFT *getOFT(int mode, MINODE *mip, int offset, int *des) {
	int i;
	for (i=0; i < NFD; i++) {
		if (running->fd[i] != 0) {
			running->fd[i] = (OFT *)malloc( sizeof(OFT) );
			running->fd[i]->mode = mode;
			running->fd[i]->inodeptr = mip;
			running->fd[i]->offset = offset;
			running->fd[i]->refcount++;
			*des = i;
			return running->fd[i];
		}
	}

}

int openValue(int ino) {
	int i;
	for (i = 0; i < NFD; i++) {
		if(running->fd[i] == 0) break;
		if(running->fd[i]->inodeptr->ino == ino) return running->fd[i]->mode;
	}

	return -1;
}

int getMode(char *mode) {
	int flag = -1;
	if ( !strcmp("RDONLY", mode) )      flag = 0;
	else if ( !strcmp("RDWR", mode) )   flag = 1;
	else if ( !strcmp("WRONLY", mode) ) flag = 2;
	else if ( !strcmp("APPEND", mode) ) flag = 3;

	return flag;
}

int _open(char *name, char *mode) {
	char ans[16];
	int fmode, offset, mino, tmode, newFD;
	MINODE *mip;
	u16 imode;
	OFT *table;

	fixPath(&name);
	mino = getino(running->cwd->dev, name);
	if(mino == 0) {
		printf("%s is not found.\n", name);
		return -1;
	}
	mip = iget(running->cwd->dev, mino);

	imode = mip->inode.i_mode;
	if ( ! FILE_MODE(imode) ) {
		printf("File is not valid to open.\n");
		return -1;
	}
	fmode = getMode(mode);
	switch(fmode) {
		case 0: 
		case 1:	 offset = 0;							  	    break;
		case 2:  offset = 0; truncateI(&mip->inode, mip->dev);  break;
		case 3:  offset = mip->inode.i_size;			  	    break;
		default: printf("%s is not a valid mode\n", mode); 	    return -1;
	}
	tmode = openValue(mip->ino);
	if (tmode != -1 && tmode != 0) {
		printf("Already opened in an incompatible mode\n");
		return -1;
	}

	table = getOFT(fmode, mip, offset, &newFD);

	mip->inode.i_atime = time(0L);
	mip->inode.i_mtime = time(0L);

	mip->dirty = 1;

	return newFD;

}

int close(int dev) {
	//this case *name needs to go through atoi in order to become the dev;
	int i;
	OFT* op;

	if(running->fd[dev] == 0) {
		printf("%d needs to be open in order to close."
			"\nPlease open %d in order to close %d\n",
			dev, dev, dev);
		return -1;
	}

	running->fd[dev] = 0;
	op = running->fd[dev];
	op->refcount--;
	if(op->refcount > 0) return 0;

	iput(op->inodeptr);
	return 0;

}

#endif
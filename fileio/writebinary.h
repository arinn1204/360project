#ifndef WRITEBINARY_H
#define WRITEBINARY_H

#include "open.h"
#include "readwrite.h"

int _cat(char *name) {
	int ino, dev, readBytes, i = 0, newdev = running->cwd->dev;
	char buf[BLKSIZE];
	//name is filename
	if(*name == 0) {
		printf("No name.\n");
		return -1;
	}
	fixPath(&name);
	ino = getino(&newdev, name);

	if (ino == 0) {
		printf("%s does not exist\n");
		return -1;
	}

	dev = _open(name, READ);


	while ( (readBytes = _read(dev, buf, BLKSIZE)) != 0) {
		while(buf[i] != 0) {
			if(buf[i] == '\n') {
				fprintf(stderr, "\r");
			}
			putchar(buf[i++]);
		}
		i = 0;
		bzero(buf, BLKSIZE);
	}

	fprintf(stderr, "\r\n");


	_close(dev);


}

int _cp(char *name) {

	char *temp;
	char buf[BLKSIZE];
	int des, source, fino, sino, newdev = running->cwd->dev;
	int bytes;


	if (*name == 0) {
		printf("No source to copy.\n");
		return -1;
	}

	if (*parameter == 0) {
		printf("No destination to copy too.\n");
		return -1;
	}

	temp = calloc( strlen(parameter) + 1, 1 );
	strcpy(temp, parameter);

	if (*name != '/') fixPath(&name);
	if (*parameter != '/') fixPath(&temp);

	fino = getino(&newdev, name);

	if (fino == 0) {
		printf("%s does not exist.\n", name);
		return -1;
	}

	sino = getino(&newdev, temp);
	//creates the file if it does not exist already
	if (sino == 0) {
		_creat(temp);
		
	}

	des = _open(name, READ);
	source = _open(temp, WRITE);

	while ( (bytes = _read(des, buf, BLKSIZE)) != 0)
		_write(source, buf, bytes);

	_close(des);
	_close(source);
	

}

int _mv(char *name) {
	int dev = running->cwd->dev, fino, sino, exist = 1;
	MINODE *mip, *sip;
	char *temp, *parent;
	//name == source
	//parameter == destination

	if (*name == 0) {
		printf("No file to move\n");
		return -1;
	}
	if (*parameter == 0) {
		printf("No destination to move file\n");
		return -1;
	}

	temp = (char *)calloc(sizeof(parameter) + 1,1);
	parent = (char *)calloc(sizeof(parameter) + 1,1);
	strcpy(temp, parameter);
	strcpy(parent, parameter);
	if (*name != '/') fixPath(&name);
	if (*temp != '/') { fixPath(&temp); fixPath(&parent); }

	strcpy(parameter, temp);
	fino = getino(&dev, name);

	if(fino == 0) {
		printf("%s does not exist.\n");
		return -1;
	}
	mip = iget(dev, fino);

	sino = getino(&dev, temp);

	if (sino == 0) {
		exist = 0;
	}

	parent = dirname(parent);

	sino = getino(&dev, parent);

	if (sino == 0) {
		printf("%s does not exist\n", parent);
		return -1;
	}

	sip = iget(dev, sino);

	if(sip->dev == mip->dev) {
		if (exist) _rm(temp);
		_link(name);
		_rm(name);
	}
	else {
		_cp(name);
		_rm(name);
	}



	iput(sip);    iput(mip);
	free(parent); free(temp);
	return 0;
}




#endif
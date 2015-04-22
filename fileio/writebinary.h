#ifndef WRITEBINARY_H
#define WRITEBINARY_H

#include "open.h"
#include "readwrite.h"

int _cat(char *name) {
	int ino, dev, readBytes, i = 0;
	char buf[BLKSIZE];
	//name is filename
	if(*name == 0) {
		printf("No name.\n");
		return -1;
	}
	fixPath(&name);
	ino = getino(running->cwd->dev, name);

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
	}

	fprintf(stderr, "\r\n");


	_close(dev);


}

int _cp(char *name) {

	char *temp;
	char buf[BLKSIZE];
	int des, source, fino, sino;
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

	fino = getino(running->cwd->dev, name);

	if (fino == 0) {
		printf("%s does not exist.\n", name);
		return -1;
	}

	sino = getino(running->cwd->dev, temp);
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





#endif
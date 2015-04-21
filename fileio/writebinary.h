#ifndef WRITEBINARY_H
#define WRITEBINARY_H

#include "open.h"
#include "readwrite.h"

int _cat(char *name) {
	int ino, dev, readBytes;
	char buf[BLKSIZE], c;
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

	}



	_close(dev);


}





#endif
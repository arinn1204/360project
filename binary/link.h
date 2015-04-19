#ifndef LINK_H
#define LINK_H

#include "structs.h"
#include "create.h"
#include "mkdir.h"

#define LINK_MODE 0xA1FF

int _link(char *source) {
	MINODE *mip, *sip;
	char *destParent, *destChild;
	int fino, sino;				//fino = first ino
								//sino = second ino
	u16 mode;

	fixPath(&source);

	if (*parameter == 0) {
		printf("No destination.\n");
		return -1;
	}
	if (*source == 0) {
		printf("No source.\n");
		return -1;
	}

	destChild = (char *)calloc(strlen(parameter) + 1, 1);
	destParent = (char *)calloc(strlen(parameter) + 1, 1);
	strcpy(destChild, parameter);
	strcpy(destParent, parameter);


	fino = getino(running->cwd->dev, source);
	if (fino == 0) {
		printf("%s was not found.\n", source);
		return -1;
	}

	mip = iget(running->cwd->dev, fino);
	mode = mip->inode.i_mode;

	if ( DIR_MODE(mode) ) {
		printf("Link to DIR is not allowed.\n");
		return -1;
	}

	fixPath(&destParent);
	sino = getino(mip->dev, destParent);
	if(sino != 0) {
		printf("%s already exists\n", destParent);
		return -1;
	}


	destParent = dirname(destParent); //gets the parent of the destChild
	sino = getino(mip->dev, destParent);
	if(sino == 0) {
		printf("%s does not exist or it is not on the same device as %s\n.",
			destParent, source);
		return -1;
	}

	sip = iget(mip->dev, sino);
	destChild = basename(destChild);

	entername(sip, fino, destChild);

	mip->inode.i_links_count = 1;
	mip->dirty = 1;
	sip->dirty = 1;
	iput(mip);
	iput(sip);

	//memory management shit
	free(destChild); free(destParent);

}

int _symlink(char *source) {
	MINODE *mip, *sip;
	u16 mode;
	int fino, sino;
	char *dest, *param;

	if (*parameter == 0) {
		printf("No destination.\n");
		return -1;
	}

	if (*source == 0) {
		printf("No source.\n");
		return -1;
	}

	param = (char *)calloc(strlen(parameter) + 1, 1);
	strcpy(param, parameter);

	fixPath(&source);
	fixPath(&param);

	strcpy(parameter, param);
	free(param);

	fino = getino(running->cwd->dev, source);

	if (fino == 0) {
		printf("%s does not exist\n", source);
		return -1;
	}

	mip = iget(running->cwd->dev, fino);


	dest = (char *)calloc(strlen(parameter) + 1, 1);
	strcpy(dest, parameter);

	dest = dirname(dest);

	sino = getino(mip->dev, dest);
	if (sino == 0) {
		printf("%s does not exist\n", dest);
		return -1;
	}

	_creat(parameter);

	sino = getino(mip->dev, parameter);
	sip = iget(mip->dev, sino);

	sip->inode.i_mode = LINK_MODE;

	memcpy(sip->inode.i_block, source, strlen(parameter) );

	sip->dirty = 1;
	iput(sip);
	iput(mip);


	free(dest);
}


#endif
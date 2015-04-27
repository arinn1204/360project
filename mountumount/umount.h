#ifndef UMOUNT_H
#define UMOUNT_H

#include "../binary/structs.h"
#include "../binary/binary.h"
#include "../binary/util.h"
#include "mount.h"


int umount(char *name) {
	int i, index;
	MINODE *mip;

	if( !isMounted(name, &index) ) {
		printf("%s not mounted. Please mount %s in order to unmount %s\n",
			name, name, name);
	}

	for (i = 0; i < NMINODES; i++) {
		if(minode[i].refcount == 0) break;
		else if (minode[i].dev == MT[index].dev) {
			printf("Device is busy.\n");
			return -1;
		}
	}

	mip = iget(MT[index].dev, getino(&MT[index].dev, name) );

	mip->mounted = 0;
	MT[index].dev = 0;
	iput(mip);

	return 0;

}


#endif

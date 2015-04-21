#ifndef READWRITE_H
#define READWRITE_H

int _read(int dev, char *obuf, int bytes) {
	MINODE *mip;
	int avail = -1, lbk = -1, start = -1, remain = -1, offset = -1;
	int filesize = -1, blk = -1, *dblock, *ddblock, count = 0;
	char *cq = obuf, *cp, buf[BLKSIZE];

	if (running->fd[dev]->mode != 0 && 
	running->fd[dev]->mode != 1) {
		printf("Can't read, open in wrong mode.\n");
		return -1;
	}


	mip = running->fd[dev] ->inodeptr;

	offset = running->fd[dev]->offset;
	filesize = mip->inode.i_size;

	avail = filesize - offset;


	if (avail <= 0) {
		//printf("No data to read.\n");
		return 0;
	}

	while(bytes && avail) {

		lbk = offset / BLKSIZE;
		start = offset % BLKSIZE;

		if (lbk < 12) {
			blk = mip->inode.i_block[lbk];
			getblock(mip->dev, blk, buf);
		}
		else if (lbk >= 12 && lbk < 256 + 12) {
			//single indirect blocks
			getblock(mip->dev, mip->inode.i_block[12], buf);
			dblock = (int *)buf;
			blk = *(dblock + lbk - 12);
		}
		else {
			//double indirect blocks
			getblock(mip->dev, mip->inode.i_block[13], buf);
			dblock = (int *)buf;
			getblock(mip->dev, *(dblock + ( (lbk - 268) / 256) ), buf);
			ddblock = (int *)buf;
			blk = *(ddblock + (lbk - 268) % 256);
		}

		cp = buf + start;
		remain = BLKSIZE - start;

		if (bytes <= remain) {
			//enough in block to finish
			memcpy(cq, cp, bytes);
			running->fd[dev]->offset += bytes;
			count += bytes;
			avail -= bytes; bytes = 0; remain -= bytes;
		}

		else {
			memcpy(cq, cp, remain);
			running->fd[dev]->offset += remain;
			count += remain;
			avail -= remain; remain = 0; bytes -= remain;

		}
	}

	return count;

}


int _write(int dev, char *ibuf, int bytes) {

}
#endif
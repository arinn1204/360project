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


	mip = running->fd[dev]->inodeptr;

	offset = running->fd[dev]->offset;
	filesize = mip->inode.i_size;

	avail = filesize - offset;


	if (avail <= 0) {
		//printf("No data to read.\n");
		obuf = 0;
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
			getblock(mip->dev, blk, buf);
		}
		else {
			//double indirect blocks
			getblock(mip->dev, mip->inode.i_block[13], buf);
			dblock = (int *)buf;
			getblock(mip->dev, *(dblock + ( (lbk - 268) / 256) ), buf);
			ddblock = (int *)buf;
			blk = *(ddblock + (lbk - 268) % 256);
			getblock(mip->dev, blk, buf);
		}

		cp = buf + start;
		remain = BLKSIZE - start;

		
		if (bytes > avail) {
			memcpy(cq, cp, avail);
			running->fd[dev]->offset += avail;

			count += avail;
			avail = 0;
		}

		else if (bytes <= remain) {
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
	MINODE *mip;
	int lbk = -1, start = -1, remain = -1, offset = -1;
	int filesize = -1, blk = -1, *dblock, *ddblock, count = 0;
	char *cq = ibuf, *cp, buf[BLKSIZE], wbuf[BLKSIZE], zbuf[BLKSIZE], fbuf[BLKSIZE];

	if (openValue(running->fd[dev]->inodeptr->ino) == -1 ||
		openValue(running->fd[dev]->inodeptr->ino) == 0) {
		printf("File not open for write mode.\n");
		return -1;
	}

	mip = running->fd[dev]->inodeptr;

	offset = running->fd[dev]->offset;
	filesize = mip->inode.i_size;

		while(bytes > 0) {
		bzero(zbuf, BLKSIZE);
		bzero(wbuf, BLKSIZE);
		bzero(fbuf, BLKSIZE);

		lbk = offset / BLKSIZE;
		start = offset % BLKSIZE;

		if (lbk < 12) {
			if(mip->inode.i_block[lbk] == 0) {
				mip->inode.i_block[lbk] = balloc(mip);
			

				putblock(mip->dev, mip->inode.i_block[lbk], zbuf);
			}

			blk = mip->inode.i_block[lbk];

		}
		else if (lbk >= 12 && lbk < 256 + 12) {
			//single indirect blocks
			if (mip->inode.i_block[12] == 0) {
				mip->inode.i_block[12] = balloc(mip);

				putblock(mip->dev, mip->inode.i_block[12], zbuf);
			}
			getblock(mip->dev, mip->inode.i_block[12], buf);
			dblock = (int *)buf;
			blk = *(dblock + lbk - 12);


			if(blk == 0) {
				*(dblock + lbk - 12) = balloc(mip);
				blk = *(dblock + lbk - 12);

				putblock(mip->dev, blk, zbuf);
				putblock(mip->dev, mip->inode.i_block[12], buf);
			}

		}
		else {
			//double indirect blocks
			if (mip->inode.i_block[13] == 0) {
				mip->inode.i_block[13] = balloc(mip);

				putblock(mip->dev, mip->inode.i_block[13], zbuf);
			}
			getblock(mip->dev, mip->inode.i_block[13], buf);
			dblock = (int *)buf;

			if( *(dblock + ( (lbk - 268) / 256) ) == 0) {
				*(dblock + ( (lbk - 268) / 256) ) = balloc(mip);
				blk = *(dblock + ( (lbk - 268) / 256) );

				putblock(mip->dev, blk, zbuf);
				putblock(mip->dev, mip->inode.i_block[13], buf);
			}

			getblock(mip->dev, *(dblock + ( (lbk - 268) / 256) ), wbuf);
			ddblock = (int *)wbuf;
			blk = *(ddblock + (lbk - 268) % 256);

			if ( blk == 0) {
				*(ddblock + (lbk - 268) % 256) = balloc(mip);
				blk = *(ddblock + (lbk - 268) % 256);

				putblock(mip->dev, blk, zbuf);
				putblock(mip->dev, *(dblock + ( (lbk - 268) / 256) ), wbuf);


			}
		}

		getblock(mip->dev, blk, fbuf);
		cq = fbuf + start;

		cp = ibuf;
		remain = BLKSIZE - start;

		if (bytes <= remain) {
			//enough in block to finish
			memcpy(cq, cp, bytes);
			running->fd[dev]->offset += bytes;

			if(running->fd[dev]->offset > mip->inode.i_size) {
				mip->inode.i_size = running->fd[dev]->offset;
			}

			count += bytes; bytes = 0; remain -= bytes;
		}

		else {
			memcpy(cq, cp, remain);
			running->fd[dev]->offset += remain;

			if(running->fd[dev]->offset > mip->inode.i_size) {
				mip->inode.i_size = running->fd[dev]->offset;
			}

			count += remain; remain = 0; bytes -= remain;

		}

		putblock(mip->dev, blk, cq);
	}

	mip->dirty = 1;
	return count;


}

int _lseek(int dev, int bytes) {
	int original;
	if (running->fd[dev] == 0 ) {
		printf("Device not open.\n");
		return 1;
	}

	if (bytes + running->fd[dev]->offset < running->fd[dev]->inodeptr->inode.i_size
	&& bytes + running->fd[dev]->offset >= 0) {
		original = running->fd[dev]->offset;
		running->fd[dev]->offset += bytes;
	}

	return original;
}


#endif
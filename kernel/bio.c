// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct {
  struct spinlock lock;
  struct buf buf[NBUF];
  uint numused;

  // clock head
  uint head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Initialize Clock Pointer
  bcache.head = bcache.numused = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
    b->flag = 1;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  uint nhead;
  struct buf *b;

  acquire(&bcache.lock);

  // Is the block already cached?
  // TODO: Could be a optimized with hash table
  // but may be irrelevante for performance
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached and cache is full
  if (bcache.numused == NBUF)
    panic("bget: no buffers");

  // Not cached.
  for(nhead = bcache.head; ; nhead=(nhead+1)%NBUF){
    b = &bcache.buf[nhead];

    if (b->refcnt > 0) {
      ;
    } else if (b->flag == 1) {
      b->flag = 0;
    } else {
      bcache.head = nhead;
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      b->flag = 1;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  bunpin(b);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  bcache.numused += b->refcnt == 0;
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  bcache.numused -= b->refcnt == 0;
  release(&bcache.lock);
}



// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.



struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

int refer_cnt[32768];
void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// 之前的kfree内存一定会释放，现在由于有个reference count的原因，内存不一定被释放，
// 所以对于被一定被释放的内存，就不能将对应物理页填充为垃圾
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  
  
  r = (struct run*)pa;

  acquire(&kmem.lock);
   int index= ((uint64)r-PGROUNDUP((uint64)end))/PGSIZE;
  if( refer_cnt[index] == 0){
    memset(pa, 1, PGSIZE);
      r->next = kmem.freelist;
  kmem.freelist = r;
  // printf("free pa: %p\n", pa);
  }

  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  // 只有在还有物理内存剩余时，才去更新refercnt
  if(r) {kmem.freelist = r->next;
  int index= ((uint64)r-PGROUNDUP((uint64)end))/PGSIZE;
  refer_cnt[index] = 1;}
    
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void increment(uint64 addr)
{
  acquire(&kmem.lock);
   int index= (addr-PGROUNDUP((uint64)end))/PGSIZE;
   refer_cnt[index]++;
release(&kmem.lock);

}

void decrement(uint64 addr)
{
  // printf("decrement\n");
  struct run *r = (struct run*)addr;
  acquire(&kmem.lock);
   int index= (addr-PGROUNDUP((uint64)end))/PGSIZE;
   refer_cnt[index]--;
   if(refer_cnt[index] == 0) {
         memset((void*)addr, 1, PGSIZE);
      r->next = kmem.freelist;
  kmem.freelist = r;
   }
  release(&kmem.lock);
}

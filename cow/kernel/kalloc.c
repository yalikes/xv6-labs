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
//we use four level of refrence counter table
extern refcounttable_t kreftable;
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
  struct run *r;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    memset(p, 1, PGSIZE);
    r = (struct run*)p;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
}

//alloc first page for reference counter table. itself need not to be counting.
void
reftableinit(){
  struct run *r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  kreftable = (refcounttable_t)r;
  release(&kmem.lock);
  //clear all RTE_V bit in kreftable
  for(int i=0;i<PGSIZE/64;i++){
    *((rte_t*)kreftable+i)=0;
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  
  uint64* ref_counter = walkreftable(kreftable ,(uint64)pa);

  if(*ref_counter <= 0){
    printf("kfree*(): free %p\n", pa);
    panic("kfree of non positive reference counter\n");
  }
  
  *ref_counter -= 1;

  if(*ref_counter == 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  refcounttable_t reftable = kreftable;
  struct run *temp_r;
  rte_t *rte;
  if(!r)
    return (void*)r;

  //alloc pages for reference table, itself need not to be counting.
  //below we walk four level of reference counter table for physical page counter.
  for(int level=3;level>=0; level--){
    rte = (rte_t*)&reftable[PPX(level, r)];
    if(!(*rte & RTE_V)){
      acquire(&kmem.lock);
      temp_r = kmem.freelist;
      if(temp_r)
        kmem.freelist = temp_r->next;
      release(&kmem.lock);
      memset(temp_r, 0, PGSIZE);
      *rte=PA2PTE(temp_r) | RTE_V;//this is OK, because pte has the same structure of rte
    }
    reftable = (refcounttable_t)PTE2PA(*rte);
  }
  int index = ((uint64)r >> 12)&0xff;
  uint64* ref_counter = &((uint64*)reftable)[index];
  *ref_counter = 1;//set its reference counter to 1

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void kreftableinit(){
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 0, PGSIZE); // fill with junk
  kreftable = (refcounttable_t)r;
}

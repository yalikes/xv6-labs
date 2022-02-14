#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  struct proc* p=myproc();
  vmprint(p->pagetable);
  uint64 pg_addr;
  int len;
  uint64 user_buf_va;
  uint64 mask_bits_buf[MAXPGSCAN/64];

  if(argaddr(0,&pg_addr)<0){
    return -1;
  }
  if(argint(1,&len)<0){
    return -1;
  }
  if(argaddr(2,&user_buf_va)<0){
    return -1;
  }

  if(len > MAXPGSCAN){
    return -1;
  }

  pg_addr = PGROUNDDOWN(pg_addr);
  for(int i=0;i<len;i++){
    //given virtual addr there is an assiciate physical page contain that virtual address.
    //thus, we need to check pte that point to that physical page
    pte_t *pte=walk(p->pagetable, pg_addr + PGSIZE*i, 0);
    if(pte == 0){//for invalid page
      mask_bits_buf[i/64] &= ~(1UL << (i % 64));//clear that bit
    }
    if(*pte & PTE_A){//accessed
      mask_bits_buf[i/64] |= 1UL << (i % 64);//set that bit
      *pte &= ~((uint64)PTE_A);//clear PTE_A
    }else{//not accessed
      mask_bits_buf[i/64] &= ~(1UL << (i % 64));
    }
  }
  int copy_bytes = len%8==0 ? len/8 : len/8 + 1;
  copyout(p->pagetable, user_buf_va, (char*)mask_bits_buf, copy_bytes);
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

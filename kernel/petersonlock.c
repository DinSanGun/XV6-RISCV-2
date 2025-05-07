#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "petersonlock.h"

#define NPLOCKS 15

// Global array of Peterson locks
struct petersonlock plocks[NPLOCKS];

// Called during kernel init
void
petersonlockinit(void)
{
  for (int i = 0; i < NPLOCKS; i++) {
    plocks[i].flag[0] = 0;
    plocks[i].flag[1] = 0;
    plocks[i].turn = 0;
    plocks[i].active = 0;
    plocks[i].owner = -1;
    plocks[i].name = 0;
  }
}

// Initialize an individual lock
void
peterson_init(struct petersonlock *lk, char *name)
{
  lk->flag[0] = 0;
  lk->flag[1] = 0;
  lk->turn = 0;
  lk->active = 1;
  lk->owner = -1;
  lk->name = name;
  __sync_synchronize(); // full memory barrier
}

// Acquire Peterson lock for a given process ID (0 or 1)
void
peterson_acquire(struct petersonlock *lk, int id)
{
  int other = 1 - id;
  if (!lk->active)
    panic("peterson_acquire: lock not initialized");

  __sync_synchronize();
  __sync_lock_test_and_set(&lk->flag[id], 1);
  lk->turn = other;
  __sync_synchronize();

  while (lk->flag[other] && lk->turn == other) {
    yield(); // avoid busy waiting
  }

  lk->owner = myproc()->pid;
}

// Release Peterson lock
void
peterson_release(struct petersonlock *lk, int id)
{
  if (!lk->active)
    panic("peterson_release: lock not initialized");
  if (lk->owner != myproc()->pid)
    panic("peterson_release: not lock holder");

  __sync_synchronize();
  lk->owner = -1;
  __sync_lock_release(&lk->flag[id]);
  __sync_synchronize();
}

// Deactivate a lock (mark as free)
void
peterson_destroy(struct petersonlock *lk)
{
  if (!lk->active)
    panic("peterson_free: lock already inactive");

  __sync_synchronize();
  lk->active = 0;
  lk->flag[0] = 0;
  lk->flag[1] = 0;
  lk->turn = 0;
  lk->owner = -1;
  lk->name = 0;
  __sync_synchronize();
}

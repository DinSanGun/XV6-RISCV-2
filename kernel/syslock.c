#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "petersonlock.h"

#define NPLOCKS 15

extern struct petersonlock plocks[];

// System call: int create_peterson(int id);
uint64
sys_create_peterson(void)
{
  int id;
  argint(0, &id);

  if (id < 0 || id >= NPLOCKS)
    return -1;

  if (plocks[id].active)
    return -1;

  peterson_init(&plocks[id], "userlock");
  plocks[id].owner = myproc()->pid;
  return 0;
}

// System call: int acquire_peterson(int id, int local_id);
uint64
sys_acquire_peterson(void)
{
  int id, local_id;
  argint(0, &id);
  argint(1, &local_id);

  if (id < 0 || id >= NPLOCKS || !plocks[id].active)
    return -1;

  if (local_id != 0 && local_id != 1)
    return -1;

  peterson_acquire(&plocks[id], local_id);
  return 0;
}

// System call: int release_peterson(int id, int local_id);
uint64
sys_release_peterson(void)
{
  int id, local_id;
  argint(0, &id);
  argint(1, &local_id);

  if (id < 0 || id >= NPLOCKS || !plocks[id].active)
    return -1;

  if (local_id != 0 && local_id != 1)
    return -1;

  if (plocks[id].owner != myproc()->pid)
    return -1;

  peterson_release(&plocks[id], local_id);
  return 0;
}

// System call: int delete_peterson(int id);
uint64
sys_delete_peterson(void)
{
  int id;
  argint(0, &id);

  if (id < 0 || id >= NPLOCKS || !plocks[id].active)
    return -1;

  if (plocks[id].owner != myproc()->pid)
    return -1;

  peterson_destroy(&plocks[id]);
  return 0;
}

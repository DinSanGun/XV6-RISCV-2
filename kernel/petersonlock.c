#include "types.h"
#include "riscv.h"
#include "defs.h"
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
  }
}

// Initialize an individual lock
int
peterson_create()
{
  for(int i = 0; i < NPLOCKS; i++){
    if(__sync_lock_test_and_set(&plocks[i].active, 1) == 0)
      return i;
  }
  return -1;
}

// Acquire a Peterson lock (internal implementation)
// Returns 0 on success, -1 on failure
int
peterson_acquire(int lock_id, int role)
{
  // Validate arguments
  if(lock_id < 0 || lock_id >= NPLOCKS || plocks[lock_id].active == 0 ||
     (role != 0 && role != 1)) {
    return -1;
  }
  
  // Peterson's algorithm to acquire the lock
  // Set flag to indicate intention to enter critical section
  __sync_lock_test_and_set(&plocks[lock_id].flag[role], 1); // Set to true
  
  // Give preference to the other process
  plocks[lock_id].turn = role;

  __sync_synchronize(); // Memory barrier to ensure all writes are visible
  
  // Wait until the other process doesn't want to enter or it's our turn
  while(plocks[lock_id].flag[1 - role] != 0 && plocks[lock_id].turn != 1 - role) {
    // Instead of busy-waiting, yield the CPU
    yield();
    
    // Re-sync memory before checking again to get fresh values
    __sync_synchronize();
  }
  
  return 0;
}

// Release a Peterson lock (internal implementation)
// Returns 0 on success, -1 on failure
int
peterson_release(int lock_id, int role)
{
  // Validate arguments
  if(lock_id < 0 || lock_id >= NPLOCKS || plocks[lock_id].active == 0 ||
     (role != 0 && role != 1)) {
    return -1;
  }
  
  // Release the lock by resetting our flag
  __sync_synchronize(); // Memory barrier before release
  __sync_lock_release(&plocks[lock_id].flag[role]); // Set to false
  
  return 0;
}

// Destroy a Peterson lock (internal implementation)
// Returns 0 on success, -1 on failure
int
peterson_destroy(int lock_id)
{
  // Validate arguments
  if(lock_id < 0 || lock_id >= NPLOCKS|| plocks[lock_id].active == 0) 
    return -1;
  
  // Mark the lock as no longer in use
  plocks[lock_id].active = 0;
  plocks[lock_id].flag[0] = 0;
  plocks[lock_id].flag[1] = 0;
  plocks[lock_id].turn = 0;

  return 0;
}

// Create a new Peterson lock
uint64
sys_peterson_create(void)
{
  return peterson_create();
}

// Acquire a Peterson lock
uint64
sys_peterson_acquire(void)
{
  int lock_id, role;

  argint(0, &lock_id);
  argint(1, &role);
  
  return peterson_acquire(lock_id, role);
}

// Release a Peterson lock
uint64
sys_peterson_release(void)
{
  int lock_id, role;

  argint(0, &lock_id);
  argint(1, &role);
  
  return peterson_release(lock_id, role);
}

// Destroy a Peterson lock
uint64
sys_peterson_destroy(void)
{
  int lock_id;

  argint(0, &lock_id);
  
  return peterson_destroy(lock_id);
}
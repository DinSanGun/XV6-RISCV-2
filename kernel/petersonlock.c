#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "petersonlock.h"

#define PETERSON_LOCKS_MAX_NUM 16

// Global variable of Peterson locks
struct petersonlock peterson_locks_arr[PETERSON_LOCKS_MAX_NUM];

// Called during kernel init (main) - initialize all available locks
void
petersonlockinit(void)
{
  for (int i = 0; i < PETERSON_LOCKS_MAX_NUM; i++) {
    peterson_locks_arr[i].flag[0] = 0;
    peterson_locks_arr[i].flag[1] = 0;
    peterson_locks_arr[i].turn = 0;
    peterson_locks_arr[i].active = 0;
  }
}

// Activate an individual lock 
int
peterson_create()
{
  for(int i = 0; i < PETERSON_LOCKS_MAX_NUM; i++){
    // If non-active lock is found - activate it atomically
    if(__sync_lock_test_and_set(&peterson_locks_arr[i].active, 1) == 0)
      return i;
  }
  return -1;
}

// Acquire a Peterson lock - returns 0 on success and -1 on failure
int
peterson_acquire(int lock_id, int role)
{
  // Validate arguments
  if(lock_id < 0 || lock_id >= PETERSON_LOCKS_MAX_NUM || peterson_locks_arr[lock_id].active == 0 ||
     (role != 0 && role != 1)) {
    return -1;
  }
  
  // Set flag as intention to enter the critical section
  peterson_locks_arr[lock_id].flag[role] = 1;  // Set to true (in pseudo-code)
  __sync_synchronize();
  
  // Set turn to self
  peterson_locks_arr[lock_id].turn = role;

  // Ensure all writes are visible (updated)
  __sync_synchronize(); 
  
  // Wait until other process does not have intention to enter or it's your turn
  while(peterson_locks_arr[lock_id].flag[1 - role] != 0 && peterson_locks_arr[lock_id].turn != 1 - role) {

    yield(); // Yields the CPU instead of busy-waiting 
    
    // Sync memory before re-checking condition
    __sync_synchronize();
  }
  
  return 0;
}

// Release a Peterson lock - returns 0 on success and -1 on failure
int
peterson_release(int lock_id, int role)
{
  // Validate the arguments
  if(lock_id < 0 || lock_id >= PETERSON_LOCKS_MAX_NUM 
                 || peterson_locks_arr[lock_id].active == 0 
                 || (role != 0 && role != 1)) {
    return -1;
  }
  
  // Release lock by resetting the flag
  __sync_synchronize();
  __sync_lock_release(&peterson_locks_arr[lock_id].flag[role]); // Set flag to false (0)
  
  return 0;
}

// Destroy a Peterson lock - returns 0 on success and -1 on failure
int
peterson_destroy(int lock_id)
{
  // Validate arguments
  if(lock_id < 0 || lock_id >= PETERSON_LOCKS_MAX_NUM
                 || peterson_locks_arr[lock_id].active == 0) 
    return -1;
  
  // Set lock to inactive state
  peterson_locks_arr[lock_id].active = 0;
  peterson_locks_arr[lock_id].flag[0] = 0;
  peterson_locks_arr[lock_id].flag[1] = 0;
  peterson_locks_arr[lock_id].turn = 0;

  return 0;
}

// Create a new Peterson lock - sys call
uint64
sys_peterson_create(void)
{
  return peterson_create();
}

// Acquire a Peterson lock - sys call
uint64
sys_peterson_acquire(void)
{
  int lock_id, role;

  argint(0, &lock_id);
  argint(1, &role);
  
  return peterson_acquire(lock_id, role);
}

// Release a Peterson lock - sys call
uint64
sys_peterson_release(void)
{
  int lock_id, role;

  argint(0, &lock_id);
  argint(1, &role);
  
  return peterson_release(lock_id, role);
}

// Destroy a Peterson lock - sys call
uint64
sys_peterson_destroy(void)
{
  int lock_id;

  argint(0, &lock_id);
  
  return peterson_destroy(lock_id);
}
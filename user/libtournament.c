#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PROCESSES_NUM 16
#define MAX_LOCKS_NUM 15

int locks_ids[MAX_LOCKS_NUM];     // ids of the locks created in the kernel
int my_roles[4];                  // roles per level of a process
int my_locks[4];                  // lock ids per level of a process
int current_process_index = -1;   // index in the tournament of a process
int levels_num = 0;               // number of levels in the tree

int tournament_create(int n) {

  if (n < 1 || n > MAX_PROCESSES_NUM)
     return -1;

  // Check that n is power of 2
  int temp = n;
  while (temp > 1) {

    if (temp % 2 != 0) 
      return -1;

    temp = temp / 2;
    levels_num++;
  }

  int locks_num = n - 1; // always 1 less than processes num

  for (int i = 0; i < locks_num; i++) {

    locks_ids[i] = peterson_create();
    if (locks_ids[i] < 0) // Creation of Peterson lock failed
        return -1;
  }

  // Create n processes
  current_process_index = 0;         

  for (int i = 1; i < n; i++) {
    if (fork() == 0) {  // Only parent (caller of tournament_create) creates child processes
      current_process_index = i;
      break;  // child stops forking
    }
  }

  // Compute roles and lock ids per level (for each process)
  int node = current_process_index;

  for (int level = levels_num - 1; level >= 0; level--) {

    int role = node % 2;            // similar to id ":= node mod 2" from pseudo-code
    node = node / 2;                // similar to "node := floor(node / 2)" from pseudo-code

    int lock_index = (1 << level) - 1 + node;  // Compute the lock id that the process is trying to acquire at this level

    my_roles[level] = role;                   // Role passed to peterson_acquire for this level
    my_locks[level] = locks_ids[lock_index];  // ID number of Peterson lock for this level
  }

  return current_process_index;
}

int tournament_acquire(void) {

  for (int i = levels_num - 1; i >= 0; i--) {
    if (peterson_acquire(my_locks[i], my_roles[i]) < 0)
      return -1;
  }
  return 0;
}

int tournament_release(void) {
  for (int i = 0; i < levels_num; i++) {
    if (peterson_release(my_locks[i], my_roles[i]) < 0)
      return -1;
 }
 return 0;
}
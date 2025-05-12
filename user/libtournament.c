#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_PROCESSES 16
#define MAX_LOCKS 15

int locks[MAX_LOCKS];
int my_roles[4];        // roles per level
int my_locks[4];        // lock ids per level
int my_index = -1;
int num_levels = 0;

int tournament_create(int n) {

  if (n < 1 || n > MAX_PROCESSES) 
    return -1;

  // Check if n is power of 2
  int temp = n;
  while (temp > 1) {
    if (temp % 2 != 0) return -1;
    temp /= 2;
    num_levels++;
  }

  int total_locks = (1 << num_levels) - 1;
  for (int i = 0; i < total_locks; i++) {
    locks[i] = peterson_create();
    if (locks[i] < 0) 
        return -1;
  }

  // Fork to create n processes
  for (int i = 0; i < n; i++) {
    if (i > 0 && fork() == 0) {
      my_index = i;
      break;
    }
  }
  if (my_index == -1) 
    my_index = 0;

  // Precompute roles and locks per level
  for (int level = 0; level < num_levels; level++) {
    
    int div = 1 << (num_levels - level - 1);
    int group = my_index / (1 << (num_levels - level));

    int role = (my_index / div) % 2;
    int lock_index = (1 << level) - 1 + group;

    my_roles[level] = role;
    my_locks[level] = locks[lock_index];
  }

  return my_index;
}

int tournament_acquire(void) {
  for (int i = 0; i < num_levels; i++) {
    if (peterson_acquire(my_locks[i], my_roles[i]) < 0)
      return -1;
  }
  return 0;
}

int tournament_release(void) {
  for (int i = num_levels - 1; i >= 0; i--) {
    if (peterson_release(my_locks[i], my_roles[i]) < 0)
      return -1;
 }
 return 0;
}
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int num_processes = 16;

  int id = tournament_create(num_processes);

  if (id < 0) {
    printf("Failed to create tournament tree\n");
    exit(1);
  }

  if(tournament_acquire() < 0){
    printf("Failed to acquire lock\n");
    exit(1);
  }

  // Critical section
  printf("Process [%d] - Tournament ID %d: in critical section\n", getpid(), id);

  if (tournament_release() < 0) {
    printf("Process %d failed to release lock\n", id);
    exit(1);
  }

  // Child processes exit immediately
  if (id != 0)
    exit(0);

  // Parent process waits for all children
  for (int i = 1; i < num_processes; i++) {
    wait(0);
  }

  for (int i = 0; i < num_processes - 1; i++) {
    if (peterson_destroy(i) < 0) 
      printf("Failed to destroy lock %d\n", i);
  }

  exit(0);
}
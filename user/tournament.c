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

  if (tournament_acquire() < 0) {
    printf("Process %d failed to acquire lock\n", id);
    exit(1);
  }

  // Critical section
  printf("Tournament ID %d: in critical section\n", id);

  if (tournament_release() < 0) {
    printf("Process %d failed to release lock\n", id);
    exit(1);
    }

    exit(0);
}
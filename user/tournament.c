#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("No command-line argument (num of processes) passed!\n");
    exit(1);
  }

  int num_processes = atoi(argv[1]); // Num of processes is give by the user a cmd-argument

  if (num_processes < 1 || num_processes > 16) {
    printf("Invalid command line argument: must be between 1 and 16\n");
    exit(1);
  }

  // Check if num_processes is a power of 2
  int temp = num_processes;
  while (temp % 2 == 0)
    temp /= 2;

  if (temp != 1) {
    printf("Invalid command line argument: must be a power of 2!\n");
    exit(1);
  }

  int id = tournament_create(num_processes);

  if (id < 0) {
    printf("Failed to create the tournament tree!\n");
    exit(1);
  }

  if (tournament_acquire() < 0) {
    printf("Process %d have failed to acquire the lock!\n", id);
    exit(1);
  }

  // ============ Critical section ============
  printf("Process <%d> - Tournament ID %d: in critical section\n", getpid(), id);
  // ==========================================

  if (tournament_release() < 0) {
    printf("Process %d failed to release the lock\n", id);
    exit(1);
  }

  // Child processes exit immediately after releasing the lock
  if (id != 0)
    exit(0);

  // Parent process is waiting for all child processes to finish
  for (int i = 1; i < num_processes; i++) {
    wait(0);
  }

  // Destroy all Peterson locks after use
  for (int i = 0; i < num_processes - 1; i++) {
    if (peterson_destroy(i) < 0) 
      printf("Failed to destroy lock %d\n", i);
  }

  exit(0);
}

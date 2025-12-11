# xv6 Assignment 2 — Synchronization Mechanisms  
*Kernel-Level Peterson Lock + Userspace Tournament Tree Lock*

## 📌 Overview  
This project implements two core synchronization systems in xv6:

1. A **kernel-level Peterson-style lock** providing mutual exclusion between two processes.  
2. A **userspace tournament tree lock** that scales synchronization to N processes using the kernel lock as a building block.

The work focuses on atomic operations, memory-order correctness, and multi-process coordination.

---

## 🔒 1. Kernel-Level Synchronization: Enhanced Peterson Lock  

### 📝 Purpose  
Provide a two-process mutual exclusion mechanism that:  
- Avoids busy-waiting  
- Uses `yield()` to cooperate with the scheduler  
- Enforces memory ordering with explicit barriers  
- Functions correctly on multiprocessor hardware  

### 🔧 Implementation  
- Added a new kernel-managed lock structure.  
- Initialized an array of locks during kernel boot.  
- Used atomic primitives:  
  - `__sync_lock_test_and_set`  
  - `__sync_lock_release`  
  - `__sync_synchronize`  
- Ensured correctness under CPU reordering and caching effects.

### 🛠️ System Calls
- `peterson_create()`  
- `peterson_acquire(id, role)`  
- `peterson_release(id, role)`  
- `peterson_destroy(id)`  

---

## 🌲 2. Userspace Tournament Tree Synchronization Library  

### 📝 Goal  
Extend two-process locking to **N = 2^k processes** using a binary tournament tree, where each node is a Peterson lock.

### 🔧 Implementation Highlights  
- Implemented in `libtournament.c`.  
- Created locks dynamically and mapped them to processes using bitwise calculations.  
- Assigned each process a unique index and a role at each level.  
- Ensured locks are released in reverse order of acquisition.  

### 📚 API  
```c
int tournament_create(int processes);
int tournament_acquire(void);
int tournament_release(void);
```

### 🧪 Test Program: `tournament.c`
- Accepts N (≤ 16).  
- Each process climbs its lock path to acquire the root lock.  
- Prints without overlap, demonstrating correct mutual exclusion.  
- Releases locks back down the tree.

---

## 🧠 Skills Demonstrated
- Kernel/user synchronization primitives  
- Memory ordering and atomic operations  
- Building hierarchical lock structures  
- Debugging concurrency in a minimal OS  
- Coordinating many processes via structured algorithms  

---

## 📜 License  
For academic and demonstration purposes only.

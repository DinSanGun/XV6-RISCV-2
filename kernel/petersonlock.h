// Peterson's lock for 2 processes
struct petersonlock {
    int flag[2];       // flags which represent the interest of each process (false = 0 / true = 1)
    int turn;          // A flag which represent the turn variable of the peterson algorithm (0 or 1)
    int active;        // indicates if the lock has been initialized
    
    // For debugging
    char *name;        // Name of lock
    int owner;         // Which PID is holding the lock
  };

  
  